#include "tasks/task_mnist.h"
#include "tasks/task_cifar10.h"
#include "tasks/task_cifar100.h"
#include "tasks/task_svhn.h"
#include "tasks/task_iris.h"
#include "tasks/task_wine.h"
#include "tasks/task_affine.h"
#include "tasks/task_peak2d.h"
#include "tasks/task_parity.h"
#include "tasks/task_fashion_mnist.h"
#include "logger.h"
#include <mutex>

using namespace nano;

task_factory_t& nano::get_tasks()
{
        static task_factory_t manager;

        static std::once_flag flag;
        std::call_once(flag, [] ()
        {
                manager.add<mnist_task_t>("mnist", "MNIST (1x28x28 digit classification)");
                manager.add<fashion_mnist_task_t>("fashion-mnist", "Fashion-MNIST (1x28x28 fashion article classification)");
                manager.add<cifar10_task_t>("cifar10", "CIFAR-10 (3x32x32 object classification)");
                manager.add<cifar100_task_t>("cifar100", "CIFAR-100 (3x32x32 object classification)");
                manager.add<svhn_task_t>("svhn", "SVHN (3x32x32 digit classification in the wild)");
                manager.add<iris_task_t>("iris", "IRIS (iris flower classification)");
                manager.add<wine_task_t>("wine", "WINE (wine classification)");
                manager.add<parity_task_t>("synth-parity", "synthetic: predict the parity bit");
                manager.add<affine_task_t>("synth-affine", "synthetic: predict random noisy affine transformations");
                manager.add<peak2d_task_t>("synth-peak2d", "synthetic: predict random peaks in noisy images");
        });

        return manager;
}

template <typename tvalues>
static size_t count_duplicates(const tvalues& values)
{
        size_t count = 0;
        auto it = values.begin();
        while ((it = std::adjacent_find(it, values.end())) != values.end())
        {
                ++ count;
                ++ it;
        }
        return count;
}

template <typename tvalues>
static size_t count_intersects(const tvalues& values1, const tvalues& values2)
{
        tvalues intersection;
        std::set_intersection(
                values1.begin(), values1.end(), values2.begin(), values2.end(),
                std::back_inserter(intersection));
        return intersection.size();
}

template <typename thashes>
static void add_hashes(const task_t& task, const fold_t& fold, thashes& hashes)
{
        const auto size = task.size(fold);
        for (size_t i = 0; i < size; ++ i)
        {
                hashes.push_back(task.ihash(fold, i));
        }
}

static size_t count_duplicates(const task_t& task, const size_t f)
{
        assert(f < task.fsize());

        std::vector<size_t> hashes;

        add_hashes(task, fold_t{f, protocol::train}, hashes);
        add_hashes(task, fold_t{f, protocol::valid}, hashes);
        add_hashes(task, fold_t{f, protocol::test}, hashes);

        return count_duplicates(hashes);
}

static size_t count_intersection(const task_t& task, const size_t f)
{
        assert(f < task.fsize());

        std::vector<size_t> train_hashes;
        std::vector<size_t> valid_hashes;
        std::vector<size_t> test_hashes;

        add_hashes(task, fold_t{f, protocol::train}, train_hashes);
        add_hashes(task, fold_t{f, protocol::valid}, valid_hashes);
        add_hashes(task, fold_t{f, protocol::test}, test_hashes);

        return  std::max(std::max(
                count_intersects(train_hashes, valid_hashes),
                count_intersects(valid_hashes, test_hashes)),
                count_intersects(test_hashes, train_hashes));
}

void nano::describe(const task_t& task, const string_t& name)
{
        log_info() << "task [" << name << "]: in(" << task.idims() << ") -> out(" << task.odims()
                << "), count = " << task.size() << ".";

        for (size_t f = 0; f < task.fsize(); ++ f)
        {
                log_info() << "fold [" << (1 + f) << "]: duplicates = " << count_duplicates(task, f) << ".";
                log_info() << "fold [" << (1 + f) << "]: intersections = " << count_intersection(task, f) << ".";

                for (const auto p : {protocol::train, protocol::valid, protocol::test})
                {
                        const auto fold = fold_t{f, p};
                        const auto size = task.size(fold);

                        std::map<string_t, size_t> lcounts;
                        for (size_t i = 0; i < size; ++ i)
                        {
                                lcounts[task.label(fold, i)] ++;
                        }

                        // describe each label separately
                        for (const auto& lcount : lcounts)
                        {
                                log_info() << "fold [" << (1 + f) << "," << to_string(p)
                                        << "]: label = " << lcount.first
                                        << ", count = " << lcount.second
                                        << "/" << size << "/" << task.size() << ".";
                        }
                }
        }
}

size_t nano::check_duplicates(const task_t& task)
{
        size_t max_duplicates = 0;
        for (size_t f = 0; f < task.fsize(); ++ f)
        {
                max_duplicates = std::max(max_duplicates, count_duplicates(task, f));
        }

        return max_duplicates;
}

size_t nano::check_intersection(const task_t& task)
{
        size_t max_duplicates = 0;
        for (size_t f = 0; f < task.fsize(); ++ f)
        {
                max_duplicates = std::max(max_duplicates, count_intersection(task, f));
        }

        return max_duplicates;
}
