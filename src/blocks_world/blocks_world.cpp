#include "blocks_world.h"

#include "carli/experiment.h"

int main(int argc, char **argv) {
  try {
    Carli::Experiment experiment;

    experiment.take_args(argc, argv);

    const auto output = dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value();

    experiment.standard_run([](){return std::make_shared<Blocks_World::Environment>();},
                            [](const std::shared_ptr<Carli::Environment> &env){return std::make_shared<Blocks_World::Agent>(env);},
                            [&output](const std::shared_ptr<Carli::Agent> &){}
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
