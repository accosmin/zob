#include "ncv_task_cmufaces.h"
#include "ncv_loss.h"
#include "ncv_color.h"
#include "ncv_image.h"
#include <fstream>
#include <boost/filesystem.hpp>

namespace ncv
{
        //-------------------------------------------------------------------------------------------------

        cmufaces_task_t::cmufaces_task_t(const string_t&)
                :       task_t("cmu-faces",
                               "CMU faces (face/non-face classification)")
        {
        }

        //-------------------------------------------------------------------------------------------------

        bool cmufaces_task_t::load(const string_t& dir)
        {
                const string_t train_face_dir = dir + "/train/face/";
                const string_t train_nonface_dir = dir + "/train/non-face/";
                const size_t n_train_samples = 2429 + 4548;

                const string_t test_face_dir = dir + "/test/face/";
                const string_t test_nonface_dir = dir + "/test/non-face/";
                const size_t n_test_samples = 472 + 23573;

                m_images.clear();
                m_folds.clear();

                return  load(train_face_dir, true, protocol::train) +
                        load(train_nonface_dir, false, protocol::train) == n_train_samples &&
                        load(test_face_dir, true, protocol::test) +
                        load(test_nonface_dir, false, protocol::test) == n_test_samples &&
                        build_folds(n_train_samples, n_test_samples);
        }

        //-------------------------------------------------------------------------------------------------

        size_t cmufaces_task_t::load(const string_t& dir, bool is_face, protocol p)
        {
                size_t cnt = 0;
                if (    boost::filesystem::exists(dir) &&
                        boost::filesystem::is_directory(dir))
                {
                        const boost::filesystem::directory_iterator it_dir_end;
                        for (boost::filesystem::directory_iterator it_dir(dir); it_dir != it_dir_end; ++ it_dir)
                        {
                                if (boost::filesystem::is_regular_file(it_dir->status()))
                                {
                                        const boost::filesystem::path path(*it_dir);

                                        const annotation_t anno(sample_region(0, 0),
                                                is_face ? "face" : "nonface",
                                                ncv::class_target(is_face ? 0 : 1, n_outputs()));

                                        image_t image;
                                        image.m_protocol = p;
                                        image.m_annotations.push_back(anno);
                                        if (image.load(path.string()))
                                        {
                                                m_images.push_back(image);
                                                ++ cnt;
                                        }
                                }
                        }
                }

                return cnt;
        }

        //-------------------------------------------------------------------------------------------------

        bool cmufaces_task_t::build_folds(size_t n_train, size_t n_test)
        {
                const fold_t train_fold = std::make_pair(0, protocol::train);
                m_folds[train_fold] = make_isamples(0, n_train, sample_region(0, 0));

                const fold_t test_fold = std::make_pair(0, protocol::test);
                m_folds[test_fold] = make_isamples(n_train, n_test, sample_region(0, 0));

                return true;
        }

        //-------------------------------------------------------------------------------------------------

        void cmufaces_task_t::load(const fold_t& fold, samples_t& samples) const
        {
                const isamples_t& isamples = this->fold(fold);

                samples.resize(isamples.size());
                for (size_t i = 0; i < isamples.size(); i ++)
                {
                        const isample_t& isample = isamples[i];
                        samples[i].load_gray(image(isample.m_index), isample.m_region);
                }
        }

        //-------------------------------------------------------------------------------------------------

        sample_t cmufaces_task_t::load(const isample_t& isample) const
        {
                sample_t sample;
                sample.load_gray(image(isample.m_index), isample.m_region);
                return sample;
        }

        //-------------------------------------------------------------------------------------------------
}
