// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <chrono>
#include <thread>

#include "Collectives.h"
#include "MPIBcastFabric.h"
#include "maml/maml.h"

namespace mpicommon {

  MPIBcastFabric::MPIBcastFabric(const Group &parentGroup,
                                 int sendRank,
                                 int recvRank)
      : group(parentGroup.dup()), sendRank(sendRank), recvRank(recvRank)
  {
    if (!group.valid())
      throw std::runtime_error(
          "#osp:mpi: trying to set up an MPI fabric "
          "with an invalid MPI communicator");

#ifndef NDEBUG
    int isInter = 0;
    MPI_CALL(Comm_test_inter(group.comm, &isInter));
    if (isInter) {
      assert(recvRank == 0 && sendRank == MPI_ROOT);
    }
#endif
  }

  /*! receive some block of data - whatever the sender has sent -
    and give us size and pointer to this data */
  size_t MPIBcastFabric::read(void *&mem)
  {
    uint32_t sz32 = 0;
    // Get the size of the bcast being sent to us. Only this part must be
    // non-blocking, after getting the size we know everyone will enter the
    // blocking barrier and the blocking bcast where the buffer is sent out.
    mpicommon::bcast(&sz32, 1, MPI_INT, recvRank, group.comm).wait();

    // TODO: Maybe at some point we should dump the buffer if it gets really
    // large
    buffer.resize(sz32);
    mem = buffer.data();
    mpicommon::bcast(mem, sz32, MPI_BYTE, recvRank, group.comm).wait();
    return sz32;
  }

  /*! send exact number of bytes - the fabric can do that through
    multiple smaller messages, but all bytes have to be
    delivered */
  void MPIBcastFabric::send(const void *_mem, size_t size)
  {
    assert(size < (1LL << 30));
    uint32_t sz32 = size;

    // Send the size of the bcast we're sending. Only this part must be
    // non-blocking, after getting the size we know everyone will enter the
    // blocking barrier and the blocking bcast where the buffer is sent out.
    auto sizeBcast = mpicommon::bcast(&sz32, 1, MPI_INT, sendRank, group.comm);

    void *mem     = const_cast<void *>(_mem);
    auto bufBcast = mpicommon::bcast(mem, sz32, MPI_BYTE, sendRank, group.comm);
    sizeBcast.wait();
    bufBcast.wait();
  }

}  // namespace mpicommon