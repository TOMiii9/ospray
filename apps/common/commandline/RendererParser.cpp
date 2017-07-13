// ======================================================================== //
// Copyright 2017 Intel Corporation                                         //
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

#include "RendererParser.h"

namespace commandline {

  bool DefaultRendererParser::parse(int ac, const char **&av)
  {
    for (int i = 1; i < ac; i++) {
      const std::string arg = av[i];
      if (arg == "--renderer" || arg == "-r") {
        assert(i+1 < ac);
        rendererType = av[++i];
      } else if (arg == "--spp" || arg == "-spp") {
        spp = atoi(av[++i]);
      } else if (arg == "--noshadows" || arg == "-ns") {
        shadows = 0;
      } else if (arg == "--ao-samples" || arg == "-ao") {
        aoSamples = atoi(av[++i]);
      } else if (arg == "--ao-distance" || arg == "-ao") {
        aoDistance = atof(av[++i]);
      } else if (arg == "--max-depth") {
        maxDepth = atoi(av[++i]);
      } else if (arg == "--bg-color") {
        bgColor.x = atof(av[++i]);
        bgColor.y = atof(av[++i]);
        bgColor.z = atof(av[++i]);
      }
    }

    // iw - moved finalize into create() so we can create multiple
    // renderers from one parsed set of settings
    // finalize();

    return true;
  }

  ospray::cpp::Renderer DefaultRendererParser::create()
  {
    // iw - moved finalize into create() so we can create multiple
    // renderers from one parsed set of settings
    finalize();
    return parsedRenderer;
  }

  void DefaultRendererParser::finalize()
  {
    if (rendererType.empty())
      rendererType = "scivis";

    parsedRenderer = ospray::cpp::Renderer(rendererType.c_str());

    // Set renderer defaults (if not using 'aoX' renderers)
    if (rendererType[0] != 'a' && rendererType[1] != 'o')
      {
        parsedRenderer.set("aoSamples", aoSamples);
        parsedRenderer.set("aoDistance", aoDistance);
        parsedRenderer.set("shadowsEnabled", shadows);
        parsedRenderer.set("aoTransparencyEnabled", 1);
        parsedRenderer.set("bgColor", bgColor);
      }

    parsedRenderer.set("spp", spp);
    parsedRenderer.set("maxDepth", maxDepth);

    parsedRenderer.commit();
  }

}
