#ifndef CARLI_EXPERIMENT_H
#define CARLI_EXPERIMENT_H

#include "agent.h"
#include "environment.h"
#include "experimental_output.h"
#include "utility/getopt.h"

#include <fstream>
#include <iostream>
#include <memory>

namespace Carli {

  class CARLI_LINKAGE Experiment {
  public:
    Experiment();
    virtual ~Experiment();

    void take_args(int argc, char **argv);

    void standard_run(const std::function<std::shared_ptr<Environment> ()> &make_env,
                      const std::function<std::shared_ptr<Agent> (const std::shared_ptr<Environment> &)> &make_agent,
                      const std::function<void (const std::shared_ptr<Agent> &)> &on_episode_termination);

  private:
    std::streambuf * cerr_bak;
    std::streambuf * cout_bak;
    std::ofstream cerr2file;
    std::ofstream cout2file;
  };

}

#endif
