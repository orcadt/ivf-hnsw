#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>

#include "utils.h"

namespace ivfhnsw {

    void random_subset(const float *x, float *x_out, size_t d, size_t nx, size_t sub_nx) {
        long seed = 1234;
        std::vector<int> perm(nx);
        faiss::rand_perm(perm.data(), nx, seed);

        for (size_t i = 0; i < sub_nx; i++)
            memcpy(x_out + i * d, x + perm[i] * d, sizeof(x_out[0]) * d);
    }


    float fvec_L2sqr(const float *x, const float *y, size_t d) {
        float PORTABLE_ALIGN32 TmpRes[8];
        #ifdef USE_AVX
        size_t qty16 = d >> 4;

        const float *pEnd1 = x + (qty16 << 4);

        __m256 diff, v1, v2;
        __m256 sum = _mm256_set1_ps(0);

        while (x < pEnd1) {
            v1 = _mm256_loadu_ps(x);
            x += 8;
            v2 = _mm256_loadu_ps(y);
            y += 8;
            diff = _mm256_sub_ps(v1, v2);
            sum = _mm256_add_ps(sum, _mm256_mul_ps(diff, diff));

            v1 = _mm256_loadu_ps(x);
            x += 8;
            v2 = _mm256_loadu_ps(y);
            y += 8;
            diff = _mm256_sub_ps(v1, v2);
            sum = _mm256_add_ps(sum, _mm256_mul_ps(diff, diff));
        }

        _mm256_store_ps(TmpRes, sum);
        float res = TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3] + TmpRes[4] + TmpRes[5] + TmpRes[6] + TmpRes[7];

        return (res);
        #else
        size_t qty16 = d >> 4;

        const float *pEnd1 = x + (qty16 << 4);

        __m128 diff, v1, v2;
        __m128 sum = _mm_set1_ps(0);

        while (x < pEnd1) {
            v1 = _mm_loadu_ps(x);
            x += 4;
            v2 = _mm_loadu_ps(y);
            y += 4;
            diff = _mm_sub_ps(v1, v2);
            sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));

            v1 = _mm_loadu_ps(x);
            x += 4;
            v2 = _mm_loadu_ps(y);
            y += 4;
            diff = _mm_sub_ps(v1, v2);
            sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));

            v1 = _mm_loadu_ps(x);
            x += 4;
            v2 = _mm_loadu_ps(y);
            y += 4;
            diff = _mm_sub_ps(v1, v2);
            sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));

            v1 = _mm_loadu_ps(x);
            x += 4;
            v2 = _mm_loadu_ps(y);
            y += 4;
            diff = _mm_sub_ps(v1, v2);
            sum = _mm_add_ps(sum, _mm_mul_ps(diff, diff));
        }
        _mm_store_ps(TmpRes, sum);
        float res = TmpRes[0] + TmpRes[1] + TmpRes[2] + TmpRes[3];

        return (res);
        #endif
    }

    // help function for getL2Distance, to get original vector from base vector file by
    // vector id, and store result in data
    void readBaseVec(const char *path_base, const size_t dim, const size_t vec_id, float *data) {
        auto vec_off = vec_id * (sizeof(uint32_t) + dim);

        std::ifstream fs_input(path_base, std::ios::binary);
        fs_input.seekg(vec_off, fs_input.beg);
        readXvecFvec<uint8_t>(fs_input, data, dim, 1);
        fs_input.close();
    }

    // read centroid vector from centroid file by centroid index
    void readCentroidVec(const char *path_centroid, const size_t dim, const long centroid_idx, float *data) {
        auto centroid_off = centroid_idx * (sizeof(uint32_t) + dim * sizeof(float));

        std::ifstream fs_input(path_centroid, std::ios::binary);
        fs_input.seekg(centroid_off, fs_input.beg);
        readXvec<float>(fs_input, data, dim);
        fs_input.close();
    }

    // calculate distance between query vector and vector in base vector file indicated by vector id
    float getL2Distance(const float *query, const char *path_vec, const size_t dim,
                        const long vec_id, vec_t type_v) {
        std::vector<float> iVec(dim);

        switch (type_v) {
        case base_vec:
            readBaseVec(path_vec, dim, vec_id, iVec.data());
            break;

        case centroid_vec:
            readCentroidVec(path_vec, dim, vec_id, iVec.data());
            break;

        default:
            std::cout << "Invalid vector type: " << type_v << std::endl;
            assert(0);
        }

        return fvec_L2sqr(query, iVec.data(), dim);
    }

    // show vector value in console
    // vector file path, vector dimention, vector index, vector file type
    void showVec(const char *path_vec, const size_t dim, const long vec_id, vec_t type_v) {
        std::vector<float> iVec(dim);

        switch (type_v) {
        case base_vec:
            readBaseVec(path_vec, dim, vec_id, iVec.data());
            break;

        case centroid_vec:
            readCentroidVec(path_vec, dim, vec_id, iVec.data());
            break;

        default:
            std::cout << "Invalid vector type: " << type_v << std::endl;
            assert(0);
        }

        auto vsz = iVec.size();
        std::cout << "[";
        for (auto i = 0; i < vsz; i++) {
            std::cout << iVec[i];
            if (i != vsz - 1) std::cout << " ";
        }
        std::cout << "]" << std::endl;
    }

    void traceVec(std::ofstream &file_trace, const char *path_vec, const size_t dim, const long vec_id, vec_t type_v) {
        std::vector<float> iVec(dim);

        switch (type_v) {
        case base_vec:
            readBaseVec(path_vec, dim, vec_id, iVec.data());
            break;

        case centroid_vec:
            readCentroidVec(path_vec, dim, vec_id, iVec.data());
            break;

        default:
            std::cout << "Invalid vector type: " << type_v << std::endl;
            assert(0);
        }

        auto vsz = iVec.size();
        file_trace << "[";
        for (auto i = 0; i < vsz; i++) {
            file_trace << iVec[i];
            if (i != vsz - 1) file_trace << " ";
        }
        file_trace << "]" << std::endl;
    }

    bool cmp(SearchInfo_t a, SearchInfo_t b) {
        if (a.distance > b.distance) {
            return false;
        } else if (abs(b.distance - a.distance) <= 0.001) {
            // two distance equals
            return (a.label < b.label);
        } else {
            return true;
        }
    }

    size_t base_vec_num(const char *path_base, size_t vec_dim) {
        struct stat st;
        int rc = stat(path_base, &st);
        if (rc) return 0;

        size_t rec_size = sizeof(uint32_t) + vec_dim * sizeof(uint8_t);
        size_t rec_num = st.st_size/rec_size;

        // verify file size, must be multiple of vector record size
        if (st.st_size != rec_size * rec_num) {
            std::cout << "Invalid size of file: " << path_base << std::endl;
            // need investigate, force core dump
            assert(0);
        }

        return rec_num;
    }

    static bool is_ext_match(char *file_nm, const char *file_ext) {
        char *ptr = strstr(file_nm, file_ext);
        char extend_nm[32];

        // file extend name not match
        if (ptr == NULL) return false;

        // skip file, which name same as extend name
        if (strlen(file_nm) == strlen(file_ext)) return false;

        // get file extend name
        size_t len_ext = strlen(file_ext);
        strncpy(extend_nm, ptr, len_ext);

        // make sure extend name same as given
        if (strcmp(extend_nm, file_ext) == 0)
            return true;
        else
            return false;
    }

    void get_files(const char *path_dir, const char *file_ext, std::vector<std::string> &file_list) {
        DIR *dir;
        struct dirent *ptr;

        dir = opendir(path_dir);
        if (dir == NULL) {
            std::cout << "Failed to open dir: " << path_dir << std::endl;
            return;
        }
        while((ptr = readdir(dir)) != NULL)
        {
            if (is_ext_match(ptr->d_name, file_ext) == false) continue;

            file_list.push_back(ptr->d_name);
        }
        std::sort(file_list.begin(), file_list.end());

        closedir(dir);
    }

    void check_files(const char *file_prefix, std::vector<std::string> &file_list) {
        auto sz = file_list.size();
        for (size_t i = 0; i < sz; i++) {
            const char *sfile = file_list[i].c_str();
            const char *ptr = strstr(sfile, file_prefix);
            if (ptr == NULL) assert(0);
            if (ptr != sfile) assert(0);
        }
    }

    void get_index_name(const char *path_idx, size_t idx, char *idx_name) {
    	sprintf(idx_name, "%s_%02lu%s", path_idx, idx, ".index");
    }

}
