#include "puddle_world.h"

#include "carli/experiment.h"

int main(int argc, char **argv) {
  try {
    Options &options = Options::get_global();
    Carli::Experiment experiment;

    if(experiment.take_args(argc, argv) < argc) {
      options.print_help(std::cerr);
      std::cerr << std::endl;

      std::ostringstream oss;
      oss << "Unknown trailing arguments:";
      while(options.optind < argc)
        oss << ' ' << argv[options.optind++];
      oss << std::endl;

      throw std::runtime_error(oss.str());
    }


    const auto output = dynamic_cast<const Option_Itemized &>(options["output"]).get_value();

    experiment.standard_run([](){return std::make_shared<Puddle_World::Environment>();},
                            [](const std::shared_ptr<Carli::Environment> &env){return std::make_shared<Puddle_World::Agent>(env);},
                            [&output](const std::shared_ptr<Carli::Agent> &agent){
                              if(output == "experiment") {
                                auto pwa = std::dynamic_pointer_cast<Puddle_World::Agent>(agent);
                                pwa->print_policy(std::cerr, 32);
                                //if(!dynamic_cast<const Option_Ranged<bool> &>(options["cmac"]).get_value())
                                //  pwa->print_value_function_grid(std::cerr);
                              }
                            }
                           );

    return 0;
  }
  catch(std::exception &ex) {
    std::cerr << "Exiting with exception: " << ex.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Exiting with unknown exception." << std::endl;
  }

  return -1;
}
