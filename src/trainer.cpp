#include <mutex>
#include "trainers/trainer_batch.h"
#include "trainers/trainer_stoch.h"

using namespace nano;

trainer_factory_t& nano::get_trainers()
{
        static trainer_factory_t manager;

        static std::once_flag flag;
        std::call_once(flag, [&m = manager] ()
        {
                m.add<batch_trainer_t>("batch", "batch trainer");
                m.add<stoch_trainer_t>("stoch", "stochastic trainer");
        });

        return manager;
}