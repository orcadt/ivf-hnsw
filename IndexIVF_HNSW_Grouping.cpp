#include "IndexIVF_HNSW_Grouping.h"
#include <algorithm>

#define TRACE_NEIGHBOUR

namespace ivfhnsw
{
    static FILE *fp_centriod = NULL;
    static const char *log_centriod = "centriod.log";

    int centriodTraceSetup()
    {
        fp_centriod = fopen(log_centriod, "w");
        if (fp_centriod == NULL)
            return -1;

        return 0;
    }

    void centriodTraceClose()
    {
        if (fp_centriod != NULL)
            fclose(fp_centriod);
    }

#if 1
#include <arpa/inet.h>
using std::cout;
using std::endl;

Index_DB::Index_DB(char *host, uint32_t port, char *db_nm, char *db_usr, char *pwd_usr) {
    sprintf(conninfo, "host=%s port=%u dbname=%s user=%s password=%s",
            host, port, db_nm, db_usr, pwd_usr);
}

Index_DB::~Index_DB() {
	if (conn) PQfinish(conn);
}

int Index_DB::Connect() {
	int rc = 0;
	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK) {
		cout << "connect failed. PQstatus : " << PQstatus(conn)  << endl;
		cout << PQerrorMessage(conn) << endl;
		PQfinish(conn);
		conn = nullptr;
        rc = -1;
	}

	return rc;
}



int Index_DB::CreateTable(const char *cmd_str, const char *table_nm) {
	PGresult   *res;
	int rc = 0;

	cout << "Execute command: " << cmd_str << endl;
    res = PQexecParams(conn,
                       cmd_str,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       1);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
    	cout << "Failed to create table: " << table_nm << endl;
    	cout << PQerrorMessage(conn) << endl;
    	rc = -1;
    }
    PQclear(res);

    return rc;
}

int Index_DB::CreateBaseTables(size_t batch_idx) {
	PGresult *res;
	char sql_str[1024], tbl_nm[128];
	int rc = -1;

    // create vector index table
	sprintf(tbl_nm, "bigann_base_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, id bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

}

int Index_DB::CreatePrecomputedIndexTables(size_t batch_idx) {
	PGresult *res;
	char sql_str[1024], tbl_nm[128];
	int rc = -1;

    // create vector index table
	sprintf(tbl_nm, "precomputed_idxs_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, id bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

}

int Index_DB::CreateIndexTables(size_t batch_idx) {
	PGresult *res;
	char sql_str[1024], tbl_nm[128];
	int rc = -1;

    // create vector index table
	sprintf(tbl_nm, "index_vector_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, id bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

    // create PQ codec table
	sprintf(tbl_nm, "pq_codec_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, codes bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

    // create norm PQ codec table
	sprintf(tbl_nm, "norm_codec_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, norm_codes bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

    // create NN centriods index table
	sprintf(tbl_nm, "nn_centroid_idxs_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, nn_centroid_idxs bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

    // create group size table
	sprintf(tbl_nm, "subgroup_sizes_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, subgroup_sizes bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

    // create inter centroid distances table
	sprintf(tbl_nm, "inter_centroid_dists_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (dim INTEGER, inter_centroid_dists bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;

    // create msic table for alphas and centroid_norms
	sprintf(tbl_nm, "misc_%lu", batch_idx);
	sprintf(sql_str,
			"CREATE TABLE %s (size INTEGER, misc_data bytea)",
			tbl_nm);
	rc = CreateTable(sql_str, tbl_nm);
	if (rc) return rc;
}

int Index_DB::DropTable(char *tbl_nm) {
	char sql_str[1024];
	int rc = 0;

	// drop index meta table
	sprintf(sql_str, "DROP TABLE IF EXISTS %s", tbl_nm);
    PGresult *res = PQexec(conn, sql_str);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    	cout << "Failed to drop table: " << tbl_nm << endl;
    	rc = -1;
    }
    PQclear(res);

    return rc;
}

int Index_DB::DropServiceTables() {
	int rc;

	rc = DropTable("system");
	if (rc) return rc;

	rc = DropTable("index_meta");
	if (rc) return rc;
}

int Index_DB::DropBaseTable(size_t batch_idx, bool drop_older) {
	char tbl_nm[128];
	int rc;

    // drop base table of given batch
    sprintf(tbl_nm, "bigann_base_%lu", batch_idx);
	rc = DropTable(tbl_nm);
	if (rc) return rc;

	// drop previous base table, when we disuse old vecotr data anymore
	if (drop_older) {
		for (size_t i = 0; i < batch_idx; i++) {
		    sprintf(tbl_nm, "bigann_base_%lu", i);
			rc = DropTable(tbl_nm);
			if (rc) return rc;
		}
	}

	return rc;
}

int Index_DB::DropPrecomputedIndexTable(size_t batch_idx, bool drop_older) {
	char tbl_nm[128];
	int rc;

    // drop precomputed index table of given batch
    sprintf(tbl_nm, "precomputed_idxs_%lu", batch_idx);
	rc = DropTable(tbl_nm);
	if (rc) return rc;

	// drop previous precomputed index table, when we disuse old vecotr data anymore
	if (drop_older) {
		for (size_t i = 0; i < batch_idx; i++) {
		    sprintf(tbl_nm, "precomputed_idxs_%lu", i);
			rc = DropTable(tbl_nm);
			if (rc) return rc;
		}
	}

	return rc;
}

void Index_DB::DropIndexTables(size_t batch_idx) {
	char tbl_nm[128];
	int rc = -1;

    // drop index vector table
    sprintf(tbl_nm, "index_vector_%lu", batch_idx);
    DropTable(tbl_nm);

    // drop PQ codec table
    sprintf(tbl_nm, "pq_codec_%lu", batch_idx);
	DropTable(tbl_nm);

    // drop norm PQ codec table
    sprintf(tbl_nm, "norm_codec_%lu", batch_idx);
	DropTable(tbl_nm);

    // drop NN centriods index table
    sprintf(tbl_nm, "nn_centroid_idxs_%lu", batch_idx);
	DropTable(tbl_nm);

    // drop NN group size table
    sprintf(tbl_nm, "subgroup_sizes_%lu", batch_idx);
	DropTable(tbl_nm);

    // drop inter centroid distances table
    sprintf(tbl_nm, "inter_centroid_dists_%lu", batch_idx);
	DropTable(tbl_nm);

    // drop msic table for alphas and centroid_norms
    sprintf(tbl_nm, "misc_%lu", batch_idx);
	DropTable(tbl_nm);
}


/*
 * system (batch_idx INTEGER)
 * index_meta (dim INTEGER, nc INTEGER, nsubc INTEGER)
*/
int Index_DB::CreateServiceTable() {
	int rc = 0;
	PGresult *res;

	rc = CreateTable("CREATE TABLE IF NOT EXISTS system (batch_idx INTEGER NOT NULL)", "system");
	if (rc) goto out;

	rc = CreateTable("CREATE TABLE IF NOT EXISTS index_meta (dim INTEGER, nc INTEGER, nsubc INTEGER)", "index_meta");
out:
	return rc;
}

template<typename T>
int Index_DB::ReadVectors(char *table_nm, size_t num_centroids, std::vector<std::vector<idx_t>> &dvec) {
	PGresult* res;
	char sql_str[512];
	sprintf(sql_str, "DECLARE mycursor CURSOR FOR select * from %s", table_nm);
	res = PQexec(conn, sql_str);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
    	std::cout << "DECLARE CURSOR failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }
    PQclear(res);

    // get all result from database
    res = PQexec(conn, "FETCH ALL in mycursor");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cout << "FETCH ALL failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }

    int nFields = PQnfields(res);
    int nRows = PQntuples(res);
    if ((size_t)nRows != num_centroids) {
    	std::cout << "Not expected records count: " << num_centroids << " in table: " << table_nm << std::endl;
        PQclear(res);
        return -1;
    }

    dvec.resize(nRows);
    for (int i = 0; i < nRows; i++)
    {
    	auto ivec = dvec[i];
    	auto vsz = atoi(PQgetvalue(res, i, 0));
    	ivec.resize(vsz);

    	/*
    	 * The binary representation of BYTEA is a bunch of bytes, which could
    	 * include embedded nulls so we have to pay attention to field length.
    	 */
    	auto bptr = PQgetvalue(res, i, 1);
    	auto blen = PQgetlength(res, i, 1);
    	memcpy(ivec.data(), bptr, blen);
    }

    PQclear(res);
}

template<typename T>
int Index_DB::WriteVector(char *table_nm, std::vector<T> &ivec) {
	int rc = 0;
	size_t sz = ivec.size();
	size_t dsize = sz * sizeof(T);

    PGresult* res;
    const uint32_t sz_big_endian = htonl((uint32_t)sz);
    const char* const paramValues[] = {
    		                               reinterpret_cast<const char* const>(&sz_big_endian),
                                           reinterpret_cast<const char* const>(ivec.data())
                                      };
    const int paramLenghts[] = {sizeof(sz_big_endian), dsize};
    const int paramFormats[] = {1, 1}; /* binary */
    char sql_str[512];
    sprintf(sql_str,
    		"INSERT INTO %s (dim, id) VALUES ($1::integer, $2::bytea)",
			table_nm);

    res = PQexecParams(
      conn,
	  sql_str,
      2,
      NULL, /* Types of parameters, unused as casts will define types */
      paramValues,
      paramLenghts,
      paramFormats,
      1 // binary results
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
    	cout << "Failed to insert data to table: " << table_nm << endl;
    	rc = -1;
    }
    PQclear(res);

    return rc;
}

int Index_DB::WriteIndexMeta(size_t dim, size_t nc, size_t nsubc) {
	int rc = -1;
	PGresult *res;
	char sql_str[512];

    sprintf(sql_str, "INSERT index_meta SET dim=%u nc=%lu nsubc=%lu",
    		dim, nc, nsubc);
    res = PQexec(conn, sql_str);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    	cout << "INSERT command failed" << endl;
        goto out;
    }
    rc = 0;

out:
    PQclear(res);
    return rc;
}

int Index_DB::LoadIndexMeta(size_t &dim, size_t &nc, size_t &nsubc) {
	PGresult* res;

	res = PQexec(conn, "DECLARE mycursor CURSOR FOR select * from index_meta");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
    	std::cout << "DECLARE CURSOR failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }
    PQclear(res);

    // get all result from database
    res = PQexec(conn, "FETCH ALL in mycursor");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cout << "FETCH ALL failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }

    int nFields = PQnfields(res);
    int nRows = PQntuples(res);
    if (nRows != 1) {
    	std::cout << "Not expected records count in table: index_meta" << std::endl;
        PQclear(res);
        return -1;
    }
    if (nFields != 3) {
    	std::cout << "Not expected fields number in table: index_meta" << std::endl;
        PQclear(res);
        return -1;
    }

    dim = atoi(PQgetvalue(res, 0, PQfnumber(res, "dim")));
    nc = atoi(PQgetvalue(res, 0, PQfnumber(res, "nc")));
    nsubc = atoi(PQgetvalue(res, 0, PQfnumber(res, "nsubc")));

    PQclear(res);

    return 0;
}

int Index_DB::GetLatestBatch(size_t &batch_idx) {
	PGresult* res;

	res = PQexec(conn, "DECLARE mycursor CURSOR FOR select * from system");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
    	std::cout << "DECLARE CURSOR failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }
    PQclear(res);

    // get all result from database
    res = PQexec(conn, "FETCH ALL in mycursor");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cout << "FETCH ALL failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }

    int nFields = PQnfields(res);
    int nRows = PQntuples(res);
    if (nRows != 1) {
    	std::cout << "Not expected records count in table: system" << std::endl;
        PQclear(res);
        return -1;
    }
    if (nFields != 1) {
    	std::cout << "Not expected fields number in table: system" << std::endl;
        PQclear(res);
        return -1;
    }

    batch_idx = atoi(PQgetvalue(res, 0, PQfnumber(res, "batch_idx")));
    PQclear(res);

    return 0;
}

/*
 * CREATE TABLE system (batch_idx INTEGER PRIMARY KEY);
*/
int Index_DB::UpdateIndex(size_t batch_idx) {
	int rc = -1;
	PGresult *res;
	char sql_str[512];

    res = PQexec(conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    	cout << "BEGIN command failed" << endl;
        PQclear(res);
        return rc;
    }
    PQclear(res);

    sprintf(sql_str, "INSERT system SET batch_idx=%u", batch_idx);
    res = PQexec(conn, sql_str);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    	cout << "INSERT command failed" << endl;
        PQclear(res);
        return rc;
    }
    PQclear(res);

    sprintf(sql_str, "DELETE FROM system WHERE batch_idx<%u", batch_idx);
    res = PQexec(conn, sql_str);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    	cout << "INSERT command failed" << endl;
        PQclear(res);
        return rc;
    }
    PQclear(res);

    res = PQexec(conn, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    	cout << "COMMIT command failed" << endl;
        PQclear(res);
        return rc;
    }
    PQclear(res);

    rc = 0;
    return rc;
}
#endif
    //================================================
    // IVF_HNSW + grouping( + pruning) implementation
    //================================================
    IndexIVF_HNSW_Grouping::IndexIVF_HNSW_Grouping(size_t dim, size_t ncentroids, size_t bytes_per_code,
                                                   size_t nbits_per_idx, size_t nsubcentroids):
           IndexIVF_HNSW(dim, ncentroids, bytes_per_code, nbits_per_idx), nsubc(nsubcentroids)
    {
        alphas.resize(nc);
        nn_centroid_idxs.resize(nc);
        subgroup_sizes.resize(nc);

        query_centroid_dists.resize(nc);
        std::fill(query_centroid_dists.begin(), query_centroid_dists.end(), 0);
        inter_centroid_dists.resize(nc);
    }


    // try to connect database
    int IndexIVF_HNSW_Grouping::setup_db(char *host, uint32_t port, char *db_nm, char *db_usr, char *pwd_usr)
    {
    	int rc;

    	db_p = new Index_DB(host, port, db_nm, db_usr, pwd_usr);
    	rc = db_p->Connect();
    	if (rc) {
    		delete db_p;
    		db_p = nullptr;
    	}

    	return rc;
    }

    void IndexIVF_HNSW_Grouping::add_group(size_t centroid_idx, size_t group_size,
                                           const float *data, const idx_t *idxs)
    {
        // Find NN centroids to source centroid 
        const float *centroid = quantizer->getDataByInternalId(centroid_idx);
        std::priority_queue<std::pair<float, idx_t>> nn_centroids_raw = quantizer->searchKnn(centroid, nsubc + 1);

        std::vector<float> centroid_vector_norms_L2sqr(nsubc);
        nn_centroid_idxs[centroid_idx].resize(nsubc);
        while (nn_centroids_raw.size() > 1) {
            centroid_vector_norms_L2sqr[nn_centroids_raw.size() - 2] = nn_centroids_raw.top().first;
            nn_centroid_idxs[centroid_idx][nn_centroids_raw.size() - 2] = nn_centroids_raw.top().second;

            if (fp_centriod) {
                // if centriod info trace enabled, log it
                fprintf(fp_centriod, "centroid index:\t%u\tsub centroid distance:\t%f\n",
                        centroid_idx, centroid_vector_norms_L2sqr[nn_centroids_raw.size() - 2]);
            }

            nn_centroids_raw.pop();
        }
        if (group_size == 0)
            return;

        const float *centroid_vector_norms = centroid_vector_norms_L2sqr.data();
        const idx_t *nn_centroids = nn_centroid_idxs[centroid_idx].data();

        // Compute centroid-neighbor_centroid and centroid-group_point vectors
        std::vector<float> centroid_vectors(nsubc * d);
        for (size_t subc = 0; subc < nsubc; subc++) {
            const float *neighbor_centroid = quantizer->getDataByInternalId(nn_centroids[subc]);
            faiss::fvec_madd(d, neighbor_centroid, -1., centroid, centroid_vectors.data() + subc * d);
        }

        // Compute alpha for group vectors
        alphas[centroid_idx] = compute_alpha(centroid_vectors.data(), data, centroid,
                                             centroid_vector_norms, group_size);

        // Compute final subcentroids
        std::vector<float> subcentroids(nsubc * d);
        for (size_t subc = 0; subc < nsubc; subc++) {
            const float *centroid_vector = centroid_vectors.data() + subc * d;
            float *subcentroid = subcentroids.data() + subc * d;
            faiss::fvec_madd(d, centroid, alphas[centroid_idx], centroid_vector, subcentroid);
        }

        // Find subcentroid idx
        std::vector<idx_t> subcentroid_idxs(group_size);
        compute_subcentroid_idxs(subcentroid_idxs.data(), subcentroids.data(), data, group_size);

        // Compute residuals
        std::vector<float> residuals(group_size * d);
        compute_residuals(group_size, data, residuals.data(), subcentroids.data(), subcentroid_idxs.data());

        // Rotate residuals
        if (do_opq){
            std::vector<float> copy_residuals(group_size * d);
            memcpy(copy_residuals.data(), residuals.data(), group_size * d * sizeof(float));
            opq_matrix->apply_noalloc(group_size, copy_residuals.data(), residuals.data());
        }

        // Compute codes
        std::vector<uint8_t> xcodes(group_size * code_size);
        pq->compute_codes(residuals.data(), xcodes.data(), group_size);

        // Decode codes
        std::vector<float> decoded_residuals(group_size * d);
        pq->decode(xcodes.data(), decoded_residuals.data(), group_size);

        // Reverse rotation
        if (do_opq){
            std::vector<float> copy_decoded_residuals(group_size * d);
            memcpy(copy_decoded_residuals.data(), decoded_residuals.data(), group_size * d * sizeof(float));
            opq_matrix->transform_transpose(group_size, copy_decoded_residuals.data(), decoded_residuals.data());
        }

        // Reconstruct data
        std::vector<float> reconstructed_x(group_size * d);
        reconstruct(group_size, reconstructed_x.data(), decoded_residuals.data(),
                    subcentroids.data(), subcentroid_idxs.data());

        // Compute norms 
        std::vector<float> norms(group_size);
        faiss::fvec_norms_L2sqr(norms.data(), reconstructed_x.data(), d, group_size);

        // Compute norm codes
        std::vector<uint8_t> xnorm_codes(group_size);
        norm_pq->compute_codes(norms.data(), xnorm_codes.data(), group_size);

        // Distribute codes
        std::vector<std::vector<idx_t> > construction_ids(nsubc);
        std::vector<std::vector<uint8_t> > construction_codes(nsubc);
        std::vector<std::vector<uint8_t> > construction_norm_codes(nsubc);
        for (size_t i = 0; i < group_size; i++) {
            idx_t idx = idxs[i];
            idx_t subcentroid_idx = subcentroid_idxs[i];

            construction_ids[subcentroid_idx].push_back(idx);
            construction_norm_codes[subcentroid_idx].push_back(xnorm_codes[i]);
            for (size_t j = 0; j < code_size; j++)
                construction_codes[subcentroid_idx].push_back(xcodes[i * code_size + j]);
        }
        // Add codes to the index
        for (size_t subc = 0; subc < nsubc; subc++) {
            idx_t subgroup_size = construction_norm_codes[subc].size();
            subgroup_sizes[centroid_idx].push_back(subgroup_size);

            for (size_t i = 0; i < subgroup_size; i++) {
                ids[centroid_idx].push_back(construction_ids[subc][i]);
                for (size_t j = 0; j < code_size; j++)
                    codes[centroid_idx].push_back(construction_codes[subc][i * code_size + j]);
                norm_codes[centroid_idx].push_back(construction_norm_codes[subc][i]);
            }
        }
    }

    /** Search procedure
      *
      * During the IVF-HNSW-PQ + Grouping search we compute
      *
      *  d = || x - y_S - y_R ||^2
      *
      * where x is the query vector, y_S the coarse sub-centroid, y_R the
      * refined PQ centroid. The expression can be decomposed as:
      *
      *  d = (1 - α) * (|| x - y_C ||^2 - || y_C ||^2) + α * (|| x - y_N ||^2 - || y_N ||^2) + || y_S + y_R ||^2 - 2 * (x|y_R)
      *      -----------------------------------------   -----------------------------------   -----------------   -----------
      *                         term 1                                 term 2                        term 3          term 4
      *
      * We use the following decomposition:
      * - term 1 is the distance to the coarse centroid, that is computed
      *   during the 1st stage search in the HNSW graph, minus the norm of the coarse centroid.
      * - term 2 is the distance to y_N one of the <subc> nearest centroids,
      *   that is used for the sub-centroid computation, minus the norm of this centroid.
      * - term 3 is the L2 norm of the reconstructed base point, that is computed at construction time, quantized
      *   using separately trained product quantizer for such norms and stored along with the residual PQ codes.
      * - term 4 is the classical non-residual distance table.
      *
      * Norms of centroids are precomputed and saved without compression, as their memory consumption is negligible.
      * If it is necessary, the norms can be added to the term 3 and compressed to byte together. We do not think that
      * it will lead to considerable decrease in accuracy.
      *
      * Since y_R defined by a product quantizer, it is split across
      * sub-vectors and stored separately for each sub-vector.
    */
    void IndexIVF_HNSW_Grouping::search(size_t k, const float *x, float *distances, long *labels)
    {
        // Distances to subcentroids. Used for pruning.
        std::vector<float> query_subcentroid_dists;

        // Indices of coarse centroids, which distances to the query are computed during the search time
        std::vector<idx_t> used_centroid_idxs;
        used_centroid_idxs.reserve(nsubc * nprobe);
        idx_t centroid_idxs[nprobe]; // Indices of the nearest coarse centroids

        // For correct search using OPQ rotate a query
        const float *query = (do_opq) ? opq_matrix->apply(1, x) : x;

#ifdef TRACE_CENTROIDS
        trace_query_centroid_dists.clear();
        trace_centroid_idxs.clear();
#endif

        // Find the nearest coarse centroids to the query
        auto coarse = quantizer->searchKnn(query, nprobe);
        for (int_fast32_t i = nprobe - 1; i >= 0; i--) {
            idx_t centroid_idx = coarse.top().second;
            centroid_idxs[i] = centroid_idx;
            query_centroid_dists[centroid_idx] = coarse.top().first;
            used_centroid_idxs.push_back(centroid_idx);

#ifdef TRACE_CENTROIDS
            trace_query_centroid_dists.push_back(coarse.top().first);
            trace_centroid_idxs.push_back(coarse.top().second);
#endif

            coarse.pop();
        }

        // Computing threshold for pruning
        float threshold = 0.0;
        if (do_pruning) {
            size_t ncode = 0;
            size_t nsubgroups = 0;

            query_subcentroid_dists.resize(nsubc * nprobe);
            float *qsd = query_subcentroid_dists.data();

            for (size_t i = 0; i < nprobe; i++) {
                const idx_t centroid_idx = centroid_idxs[i];
                const size_t group_size = norm_codes[centroid_idx].size();
                if (group_size == 0)
                    continue;

                const float alpha = alphas[centroid_idx];
                const float term1 = (1 - alpha) * query_centroid_dists[centroid_idx];

                for (size_t subc = 0; subc < nsubc; subc++) {
                    if (subgroup_sizes[centroid_idx][subc] == 0)
                        continue;

                    const idx_t nn_centroid_idx = nn_centroid_idxs[centroid_idx][subc];
                    // Compute the distance to the coarse centroid if it is not computed
                    if (query_centroid_dists[nn_centroid_idx] < EPS) {
                        const float *nn_centroid = quantizer->getDataByInternalId(nn_centroid_idx);
                        query_centroid_dists[nn_centroid_idx] = fvec_L2sqr(query, nn_centroid, d);
                        used_centroid_idxs.push_back(nn_centroid_idx);
                    }
                    qsd[subc] = term1 - alpha * ((1 - alpha) * inter_centroid_dists[centroid_idx][subc]
                                                 - query_centroid_dists[nn_centroid_idx]);
                    threshold += qsd[subc];
                    nsubgroups++;
                }
                ncode += group_size;
                qsd += nsubc;
                if (ncode >= 2 * max_codes)
                    break;
            }
            threshold /= nsubgroups;
        }

        // Precompute table
        pq->compute_inner_prod_table(query, precomputed_table.data());

        // Prepare max heap with k answers
        faiss::maxheap_heapify(k, distances, labels);

        size_t ncode = 0;
        const float *qsd = query_subcentroid_dists.data();

#ifdef TRACE_NEIGHBOUR
        std::vector<float> query_vector_dists;
        char *neighbour_log = "neighbour_hit.log";
        std::ofstream log_trace;
        log_trace.open(neighbour_log, std::ofstream::out | std::ofstream::app);
        if (!log_trace.is_open()) {
            std::cout << "Failed to open log file for neighbour traceing" << std::endl;
        }
#endif

        for (size_t i = 0; i < nprobe; i++) {
            const idx_t centroid_idx = centroid_idxs[i];
            const size_t group_size = norm_codes[centroid_idx].size();
            if (group_size == 0)
                continue;

            const float alpha = alphas[centroid_idx];
            const float term1 = (1 - alpha) * (query_centroid_dists[centroid_idx] - centroid_norms[centroid_idx]);

            const uint8_t *code = codes[centroid_idx].data();
            const uint8_t *norm_code = norm_codes[centroid_idx].data();
            const idx_t *id = ids[centroid_idx].data();

#ifdef TRACE_NEIGHBOUR
            log_trace << "centroid " << centroid_idx << " with threshold: " << threshold;
            log_trace << " get neighbours distance:\n";
            query_vector_dists.clear();
#endif

            for (size_t subc = 0; subc < nsubc; subc++) {
                const size_t subgroup_size = subgroup_sizes[centroid_idx][subc];
                if (subgroup_size == 0)
                    continue;

                // Check pruning condition
                if (!do_pruning || qsd[subc] < threshold) {
                    const idx_t nn_centroid_idx = nn_centroid_idxs[centroid_idx][subc];

                    // Compute the distance to the coarse centroid if it is not computed
                    if (query_centroid_dists[nn_centroid_idx] < EPS) {
                        const float *nn_centroid = quantizer->getDataByInternalId(nn_centroid_idx);
                        query_centroid_dists[nn_centroid_idx] = fvec_L2sqr(query, nn_centroid, d);
                        used_centroid_idxs.push_back(nn_centroid_idx);
                    }

                    const float term2 = alpha * (query_centroid_dists[nn_centroid_idx] - centroid_norms[nn_centroid_idx]);
                    norm_pq->decode(norm_code, norms.data(), subgroup_size);

                    for (size_t j = 0; j < subgroup_size; j++) {
                        const float term4 = 2 * pq_L2sqr(code + j * code_size);
                        const float dist = term1 + term2 + norms[j] - term4; //term3 = norms[j]
#ifdef TRACE_NEIGHBOUR
                        if (log_trace.is_open()) {
                            query_vector_dists.push_back(dist);
                        }
#endif
                        if (dist < distances[0]) {
                            faiss::maxheap_pop(k, distances, labels);
                            faiss::maxheap_push(k, distances, labels, dist, id[j]);
                        }
                    }
                    ncode += subgroup_size;
                }
                // Shift to the next group
                code += subgroup_size * code_size;
                norm_code += subgroup_size;
                id += subgroup_size;
            }
#ifdef TRACE_NEIGHBOUR
            if (log_trace.is_open()) {
                std::sort(query_vector_dists.begin(), query_vector_dists.end());
                for (auto it = query_vector_dists.begin(); it < query_vector_dists.end(); it++) {
                    log_trace << *it << "\n";
                }
            }
#endif
            if (ncode >= max_codes)
                break;
            if (do_pruning)
                qsd += nsubc;
        }
#ifdef TRACE_NEIGHBOUR
        if (log_trace.is_open()) log_trace.close();
#endif
        // Zero computed dists for later queries
        for (idx_t used_centroid_idx : used_centroid_idxs)
            query_centroid_dists[used_centroid_idx] = 0;

        if (do_opq)
            delete const_cast<float *>(query);
    }

    void IndexIVF_HNSW_Grouping::searchDisk(size_t k, const float *query, float *distances,
                                            long *labels, const char *path_base)
    {
        float distances_base[2*k];
        long labels_base[2*k];
        std::vector<SearchInfo_t> searchRet(2*k);

        // search by ANN first to get 2*k result
        search(k, query, distances_base, labels_base);

        /*
         *  get vector from disk according to result's lablel value (vector index in base vector file)
         *  and recalculate exactly distance between vector and query
         */
        SearchInfo_t sret;
        for (int di = 2*k - 1; di >= 0; di--) {
            sret.distance = getL2Distance(query, path_base, d,
                    labels_base[di], base_vec);;
            sret.label = labels_base[di];
            searchRet.push_back(sret);
        }

        // sort search result by real distance then label value
        std::sort(searchRet.begin(), searchRet.end(), cmp);

        // return top k value with minimize real distance
        for (int i = 0; i < k; i++) {
            distances[i] = searchRet[i].distance;
            labels[i] = searchRet[i].label;
        }
    }

	int IndexIVF_HNSW_Grouping::prepare_db(size_t batch_idx)
    {
		if (db_p == nullptr) {
		    std::cout << "Connect to servicedb ..." << std::endl;
			int rc = setup_db("localhost", 5432, "servicedb", "postgres", "postgres");
			if (rc) return rc;
		}
		std::cout << "Prepare tables in servicedb for batch index: " << batch_idx << std::endl;
        db_p->DropIndexTables(batch_idx);
		return db_p->CreateIndexTables(batch_idx);
    }

	int IndexIVF_HNSW_Grouping::commit_db_index(size_t batch_idx)
    {
		return db_p->UpdateIndex(batch_idx);
    }

    int IndexIVF_HNSW_Grouping::write_db_index(size_t batch_idx)
    {
    	char table_nm[128];
    	int rc;

        // Save vector indices
    	sprintf(table_nm, "index_vector_%lu", batch_idx);
        for (size_t i = 0; i < nc; i++) {
        	rc = db_p->WriteVector(table_nm, ids[i]);
        	if (rc) {
        		std::cout << "Failed to save index vector" << std::endl;
        		return rc;
        	}
        }

        // Save PQ codes
    	sprintf(table_nm, "pq_codec_%lu", batch_idx);
        for (size_t i = 0; i < nc; i++) {
        	rc = db_p->WriteVector(table_nm, codes[i]);
        	if (rc) {
        		std::cout << "Failed to save PQ codes" << std::endl;
        		return rc;
        	}
        }

        // Save norm PQ codes
    	sprintf(table_nm, "norm_codec_%lu", batch_idx);
        for (size_t i = 0; i < nc; i++) {
        	rc = db_p->WriteVector(table_nm, norm_codes[i]);
        	if (rc) {
        		std::cout << "Failed to save norm PQ codes" << std::endl;
        		return rc;
        	}
        }

        // Save NN centroid indices
    	sprintf(table_nm, "nn_centroid_idxs_%lu", batch_idx);
        for (size_t i = 0; i < nc; i++) {
        	rc = db_p->WriteVector(table_nm, nn_centroid_idxs[i]);
        	if (rc) {
        		std::cout << "Failed to save NN centroid indices" << std::endl;
        		return rc;
        	}
        }

        // Save group sizes
    	sprintf(table_nm, "subgroup_sizes_%lu", batch_idx);
        for (size_t i = 0; i < nc; i++) {
        	rc = db_p->WriteVector(table_nm, subgroup_sizes[i]);
        	if (rc) {
        		std::cout << "Failed to save group sizes" << std::endl;
        		return rc;
        	}
        }

        // Save inter centroid distances
    	sprintf(table_nm, "inter_centroid_dists_%lu", batch_idx);
        for (size_t i = 0; i < nc; i++) {
        	rc = db_p->WriteVector(table_nm, inter_centroid_dists[i]);
        	if (rc) {
        		std::cout << "Failed to save inter centroid distances" << std::endl;
        		return rc;
        	}
        }

        // Save alphas and centroid norms
        sprintf(table_nm, "misc_%lu", batch_idx);
    	rc = db_p->WriteVector(table_nm, alphas);
    	if (rc) {
    		std::cout << "Failed to save alphas" << std::endl;
    		return rc;
    	}
    	rc = db_p->WriteVector(table_nm, centroid_norms);
    	if (rc) {
    		std::cout << "Failed to save centroid norms" << std::endl;
    		return rc;
    	}

    	return 0;
    }

    void IndexIVF_HNSW_Grouping::write(const char *path_index, bool do_trunc)
    {
        std::ofstream output;

        if (do_trunc)
            output.open(path_index, std::ios::binary | std::ios::trunc);
        else
            output.open(path_index, std::ios::binary);

        write_variable(output, d);
        write_variable(output, nc);
        write_variable(output, nsubc);

        // Save vector indices
        for (size_t i = 0; i < nc; i++)
            write_vector(output, ids[i]);

        // Save PQ codes
        for (size_t i = 0; i < nc; i++)
            write_vector(output, codes[i]);

        // Save norm PQ codes
        for (size_t i = 0; i < nc; i++)
            write_vector(output, norm_codes[i]);

        // Save NN centroid indices
        for (size_t i = 0; i < nc; i++)
            write_vector(output, nn_centroid_idxs[i]);

        // Write group sizes
        for (size_t i = 0; i < nc; i++)
            write_vector(output, subgroup_sizes[i]);

        // Save alphas
        write_vector(output, alphas);

        // Save centroid norms
        // centroid norms count is nc
        write_vector(output, centroid_norms);

        // Save inter centroid distances
        for (size_t i = 0; i < nc; i++)
            write_vector(output, inter_centroid_dists[i]);
    }

    void IndexIVF_HNSW_Grouping::write(const char *path_index)
    {
        this->write(path_index, false);
    }

    void IndexIVF_HNSW_Grouping::read(const char *path_index)
    {
        std::ifstream input(path_index, std::ios::binary);

        read_variable(input, d);
        read_variable(input, nc);
        read_variable(input, nsubc);

        // Read ids
        for (size_t i = 0; i < nc; i++)
            read_vector(input, ids[i]);

        // Read PQ codes
        for (size_t i = 0; i < nc; i++)
            read_vector(input, codes[i]);

        // Read norm PQ codes
        for (size_t i = 0; i < nc; i++)
            read_vector(input, norm_codes[i]);

        // Read NN centroid indices
        for (size_t i = 0; i < nc; i++)
            read_vector(input, nn_centroid_idxs[i]);

        // Read group sizes
        for (size_t i = 0; i < nc; i++)
            read_vector(input, subgroup_sizes[i]);

        // Read alphas
        read_vector(input, alphas);

        // Read centroid norms
        read_vector(input, centroid_norms);

        // Read inter centroid distances
        for (size_t i = 0; i < nc; i++)
            read_vector(input, inter_centroid_dists[i]);
    }


    void IndexIVF_HNSW_Grouping::train_pq(size_t n, const float *x)
    {
        std::vector<float> train_subcentroids;
        std::vector<float> train_residuals;

        train_subcentroids.reserve(n*d);
        train_residuals.reserve(n*d);

        std::vector<idx_t> assigned(n);
        assign(n, x, assigned.data());

        std::unordered_map<idx_t, std::vector<float>> group_map;

        for (size_t i = 0; i < n; i++) {
            const idx_t key = assigned[i];
            for (size_t j = 0; j < d; j++)
                group_map[key].push_back(x[i*d + j]);
        }

        // Train Residual PQ
        std::cout << "Training Residual PQ codebook " << std::endl;
        for (auto group : group_map) {
            const idx_t centroid_idx = group.first;
            const float *centroid = quantizer->getDataByInternalId(centroid_idx);
            const std::vector<float> data = group.second;
            const int group_size = data.size() / d;

            std::vector<idx_t> nn_centroid_idxs(nsubc);
            std::vector<float> centroid_vector_norms(nsubc);
            auto nn_centroids_raw = quantizer->searchKnn(centroid, nsubc + 1);

            while (nn_centroids_raw.size() > 1) {
                centroid_vector_norms[nn_centroids_raw.size() - 2] = nn_centroids_raw.top().first;
                nn_centroid_idxs[nn_centroids_raw.size() - 2] = nn_centroids_raw.top().second;
                nn_centroids_raw.pop();
            }

            // Compute centroid-neighbor_centroid and centroid-group_point vectors
            std::vector<float> centroid_vectors(nsubc * d);
            for (size_t subc = 0; subc < nsubc; subc++) {
                const float *nn_centroid = quantizer->getDataByInternalId(nn_centroid_idxs[subc]);
                faiss::fvec_madd(d, nn_centroid, -1., centroid, centroid_vectors.data() + subc * d);
            }

            // Find alphas for vectors
            const float alpha = compute_alpha(centroid_vectors.data(), data.data(), centroid,
                                              centroid_vector_norms.data(), group_size);

            // Compute final subcentroids 
            std::vector<float> subcentroids(nsubc * d);
            for (size_t subc = 0; subc < nsubc; subc++)
                faiss::fvec_madd(d, centroid, alpha, centroid_vectors.data() + subc*d, subcentroids.data() + subc*d);

            // Find subcentroid idx
            std::vector<idx_t> subcentroid_idxs(group_size);
            compute_subcentroid_idxs(subcentroid_idxs.data(), subcentroids.data(), data.data(), group_size);

            // Compute Residuals
            std::vector<float> residuals(group_size * d);
            compute_residuals(group_size, data.data(), residuals.data(), subcentroids.data(), subcentroid_idxs.data());

            for (size_t i = 0; i < group_size; i++) {
                const idx_t subcentroid_idx = subcentroid_idxs[i];
                for (size_t j = 0; j < d; j++) {
                    train_subcentroids.push_back(subcentroids[subcentroid_idx*d + j]);
                    train_residuals.push_back(residuals[i*d + j]);
                }
            }
        }
        // Train OPQ rotation matrix and rotate residuals
        if (do_opq){
            faiss::OPQMatrix *matrix = new faiss::OPQMatrix(d, pq->M);

            std::cout << "Training OPQ Matrix" << std::endl;
            matrix->verbose = true;
            matrix->max_train_points = n;
            matrix->niter = 100;
            matrix->train(n, train_residuals.data());
            opq_matrix = matrix;

            std::vector<float> copy_train_residuals(n * d);
            memcpy(copy_train_residuals.data(), train_residuals.data(), n * d * sizeof(float));
            opq_matrix->apply_noalloc(n, copy_train_residuals.data(), train_residuals.data());
        }

        printf("Training %zdx%zd PQ on %ld vectors in %dD\n", pq->M, pq->ksub, train_residuals.size() / d, d);
        pq->verbose = true;
        pq->train(n, train_residuals.data());

        // Norm PQ
        std::cout << "Training Norm PQ codebook " << std::endl;
        std::vector<float> train_norms;
        const float *residuals = train_residuals.data();
        const float *subcentroids = train_subcentroids.data();

        for (auto p : group_map) {
            const std::vector<float> data = p.second;
            const size_t group_size = data.size() / d;

            // Compute Codes 
            std::vector<uint8_t> xcodes(group_size * code_size);
            pq->compute_codes(residuals, xcodes.data(), group_size);

            // Decode Codes 
            std::vector<float> decoded_residuals(group_size * d);
            pq->decode(xcodes.data(), decoded_residuals.data(), group_size);

            // Reverse rotation
            if (do_opq){
                std::vector<float> copy_decoded_residuals(group_size * d);
                memcpy(copy_decoded_residuals.data(), decoded_residuals.data(), group_size * d * sizeof(float));
                opq_matrix->transform_transpose(group_size, copy_decoded_residuals.data(), decoded_residuals.data());
            }

            // Reconstruct Data 
            std::vector<float> reconstructed_x(group_size * d);
            for (size_t i = 0; i < group_size; i++)
                faiss::fvec_madd(d, decoded_residuals.data() + i*d, 1., subcentroids+i*d, reconstructed_x.data() + i*d);

            // Compute norms 
            std::vector<float> group_norms(group_size);
            faiss::fvec_norms_L2sqr(group_norms.data(), reconstructed_x.data(), d, group_size);

            for (size_t i = 0; i < group_size; i++)
                train_norms.push_back(group_norms[i]);

            residuals += group_size * d;
            subcentroids += group_size * d;
        }
        printf("Training %zdx%zd PQ on %ld vectors in 1D\n", norm_pq->M, norm_pq->ksub, train_norms.size());
        norm_pq->verbose = true;
        norm_pq->train(n, train_norms.data());
    }

    void IndexIVF_HNSW_Grouping::compute_inter_centroid_dists()
    {
        for (size_t i = 0; i < nc; i++) {
            const float *centroid = quantizer->getDataByInternalId(i);
            inter_centroid_dists[i].resize(nsubc);
            for (size_t subc = 0; subc < nsubc; subc++) {
                const idx_t nn_centroid_idx = nn_centroid_idxs[i][subc];
                const float *nn_centroid = quantizer->getDataByInternalId(nn_centroid_idx);
                inter_centroid_dists[i][subc] = fvec_L2sqr(nn_centroid, centroid, d);
            }
        }
    }

    void IndexIVF_HNSW_Grouping::dump_inter_centroid_dists(char *path)
    {
        FILE *fp;
        char buf[512];

        fp = fopen(path, "w");
        if (fp == NULL) {
            std::cout << "Failed to open file: " << path << std::endl;
            return;
        }

        for (size_t i = 0; i < nc; i++) {
            for (size_t subc = 0; subc < nsubc; subc++) {
                sprintf(buf, "distance of centriod %lu to centriod %lu is %f\n",
                        i, subc, inter_centroid_dists[i][subc]);
                fwrite(buf, 1, strlen(buf), fp);
            }
        }

        fclose(fp);
    }

    void IndexIVF_HNSW_Grouping::compute_residuals(size_t n, const float *x, float *residuals,
                                                   const float *subcentroids, const idx_t *keys)
    {
        for (size_t i = 0; i < n; i++) {
            const float *subcentroid = subcentroids + keys[i]*d;
            faiss::fvec_madd(d, x + i*d, -1., subcentroid, residuals + i*d);
        }
    }

    void IndexIVF_HNSW_Grouping::reconstruct(size_t n, float *x, const float *decoded_residuals,
                                             const float *subcentroids, const idx_t *keys)
    {
        for (size_t i = 0; i < n; i++) {
            const float *subcentroid = subcentroids + keys[i] * d;
            faiss::fvec_madd(d, decoded_residuals + i*d, 1., subcentroid, x + i*d);
        }
    }

    void IndexIVF_HNSW_Grouping::compute_subcentroid_idxs(idx_t *subcentroid_idxs, const float *subcentroids,
                                                          const float *x, size_t group_size)
    {
        for (size_t i = 0; i < group_size; i++) {
            float min_dist = 0.0;
            idx_t min_idx = -1;
            for (size_t subc = 0; subc < nsubc; subc++) {
                const float *subcentroid = subcentroids + subc * d;
                float dist = fvec_L2sqr(subcentroid, x + i*d, d);
                if (min_idx == -1 || dist < min_dist){
                    min_dist = dist;
                    min_idx = subc;
                }
            }
            subcentroid_idxs[i] = min_idx;
        }
    }

    float IndexIVF_HNSW_Grouping::compute_alpha(const float *centroid_vectors, const float *points,
                                                const float *centroid, const float *centroid_vector_norms_L2sqr,
                                                size_t group_size)
    {
        float group_numerator = 0.0;
        float group_denominator = 0.0;

        std::vector<float> point_vectors(group_size * d);
        for (size_t i = 0; i < group_size; i++)
            faiss::fvec_madd(d, points + i*d , -1., centroid, point_vectors.data() + i*d);

        for (size_t i = 0; i < group_size; i++) {
            const float *point_vector = point_vectors.data() + i * d;
            const float *point = points + i * d;

            std::priority_queue<std::pair<float, std::pair<float, float>>> maxheap;

            for (size_t subc = 0; subc < nsubc; subc++) {
                const float *centroid_vector = centroid_vectors + subc * d;

                float numerator = faiss::fvec_inner_product(centroid_vector, point_vector, d);
                numerator = (numerator > 0) ? numerator : 0.0;

                const float denominator = centroid_vector_norms_L2sqr[subc];
                const float alpha = numerator / denominator;

                std::vector<float> subcentroid(d);
                faiss::fvec_madd(d, centroid, alpha, centroid_vector, subcentroid.data());

                const float dist = fvec_L2sqr(point, subcentroid.data(), d);
                maxheap.emplace(-dist, std::make_pair(numerator, denominator));
            }

            group_numerator += maxheap.top().second.first;
            group_denominator += maxheap.top().second.second;
        }
        return (group_denominator > 0) ? group_numerator / group_denominator : 0.0;
    }
}
