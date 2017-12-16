#!/bin/bash

efConstruction="500"
M="16"
numMillions="1000"
n="${numMillions}000000"
baseDir="/home/dbaranchuk/"
k="100"
nq="10000"

d="128"
gt_d="1000"

M_PQ="8"
nprobes="32"
max_codes="10000"

nc="1"
ncentroids="993127"
efSearch="80"
nsubcentroids="64"

subdir="final_models/SIFT1B/"

# Paths
path_data="${baseDir}data/bigann/bigann_base.bvecs" 
path_edges="${baseDir}${subdir}centroids${ncentroids}_ef${efConstruction}.ivecs" 
path_info="${baseDir}${subdir}centroids${ncentroids}_ef${efConstruction}.bin" 

path_gt="${baseDir}data/bigann/gnd/idx_1000M.ivecs" 
path_q="${baseDir}data/bigann/bigann_query.bvecs"

path_pq="${baseDir}${subdir}pq${M_PQ}_nsubc${nsubcentroids}.pq"
path_norm_pq="${baseDir}${subdir}norm_pq${M_PQ}_nsubc${nsubcentroids}.pq"
path_precomputed_idxs="${baseDir}staff-ivf-hnsw/sift1B_precomputed_idxs_${ncentroids}.ivecs"
path_index="${baseDir}${subdir}hybrid${nc}M_PQ${M_PQ}_nsubc${nsubcentroids}.index"
path_learn="${baseDir}data/bigann/bigann_learn.bvecs"
path_centroids="${baseDir}data/sift1B_centroids${nc}M.fvecs"
 
path_groups="${baseDir}data/groups/sift1B_groups.dat";
path_idxs="${baseDir}data/groups/sift1B_idxs.ivecs"

#./main --help
for i in {1..5}
do
/home/dbaranchuk/ivf-hnsw/bin/demo_ivfhnsw_grouping_sift1b -path_centroids ${path_centroids} -path_learn ${path_learn} -path_index ${path_index} -path_precomputed_idx ${path_precomputed_idxs} -path_pq ${path_pq} -path_norm_pq ${path_norm_pq} -path_data ${path_data} -path_edges ${path_edges} -path_info ${path_info} -path_gt ${path_gt} -path_q ${path_q} -path_groups ${path_groups} -path_idxs ${path_idxs} -k ${k} -n ${n} -nq ${nq} -nc ${ncentroids} -M ${M} -M_PQ ${M_PQ} -efConstruction ${efConstruction} -efSearch ${efSearch} -d ${d} -gt_d ${gt_d} -nprobes ${nprobes} -max_codes ${max_codes} -nsubc ${nsubcentroids}
done

nprobes="64"
max_codes="30000"
efSearch="100"

for i in {1..5}
do
/home/dbaranchuk/ivf-hnsw/bin/demo_ivfhnsw_grouping_sift1b -path_centroids ${path_centroids} -path_learn ${path_learn} -path_index ${path_index} -path_precomputed_idx ${path_precomputed_idxs} -path_pq ${path_pq} -path_norm_pq ${path_norm_pq} -path_data ${path_data} -path_edges ${path_edges} -path_info ${path_info} -path_gt ${path_gt} -path_q ${path_q} -path_groups ${path_groups} -path_idxs ${path_idxs} -k ${k} -n ${n} -nq ${nq} -nc ${ncentroids} -M ${M} -M_PQ ${M_PQ} -efConstruction ${efConstruction} -efSearch ${efSearch} -d ${d} -gt_d ${gt_d} -nprobes ${nprobes} -max_codes ${max_codes} -nsubc ${nsubcentroids}
done 

nprobes="128" 
max_codes="50000"
efSearch="140"

for i in {1..5}
do
/home/dbaranchuk/ivf-hnsw/bin/demo_ivfhnsw_grouping_sift1b -path_centroids ${path_centroids} -path_learn ${path_learn} -path_index ${path_index} -path_precomputed_idx ${path_precomputed_idxs} -path_pq ${path_pq} -path_norm_pq ${path_norm_pq} -path_data ${path_data} -path_edges ${path_edges} -path_info ${path_info} -path_gt ${path_gt} -path_q ${path_q} -path_groups ${path_groups} -path_idxs ${path_idxs} -k ${k} -n ${n} -nq ${nq} -nc ${ncentroids} -M ${M} -M_PQ ${M_PQ} -efConstruction ${efConstruction} -efSearch ${efSearch} -d ${d} -gt_d ${gt_d} -nprobes ${nprobes} -max_codes ${max_codes} -nsubc ${nsubcentroids}
done

M_PQ="16"
nprobes="32" 
max_codes="10000"
efSearch="80"

path_edges="${baseDir}${subdir}centroids${ncentroids}_ef${efConstruction}_nsubc${nsubcentroids}_PQ${M_PQ}.ivecs"            
path_info="${baseDir}${subdir}centroids${ncentroids}_ef${efConstruction}_nsubc${nsubcentroids}_PQ${M_PQ}.bin"

path_pq="${baseDir}${subdir}pq${M_PQ}_nsubc${nsubcentroids}.pq"
path_norm_pq="${baseDir}${subdir}norm_pq${M_PQ}_nsubc${nsubcentroids}.pq"
path_index="${baseDir}${subdir}hybrid${nc}M_PQ${M_PQ}_nsubc${nsubcentroids}.index"

for i in {1..5}
do
/home/dbaranchuk/ivf-hnsw/bin/demo_ivfhnsw_grouping_sift1b -path_centroids ${path_centroids} -path_learn ${path_learn} -path_index ${path_index} -path_precomputed_idx ${path_precomputed_idxs} -path_pq ${path_pq} -path_norm_pq ${path_norm_pq} -path_data ${path_data} -path_edges ${path_edges} -path_info ${path_info} -path_gt ${path_gt} -path_q ${path_q} -path_groups ${path_groups} -path_idxs ${path_idxs} -k ${k} -n ${n} -nq ${nq} -nc ${ncentroids} -M ${M} -M_PQ ${M_PQ} -efConstruction ${efConstruction} -efSearch ${efSearch} -d ${d} -gt_d ${gt_d} -nprobes ${nprobes} -max_codes ${max_codes} -nsubc ${nsubcentroids}
done

nprobes="64" 
max_codes="30000"
efSearch="100"

for i in {1..5}
do
/home/dbaranchuk/ivf-hnsw/bin/demo_ivfhnsw_grouping_sift1b -path_centroids ${path_centroids} -path_learn ${path_learn} -path_index ${path_index} -path_precomputed_idx ${path_precomputed_idxs} -path_pq ${path_pq} -path_norm_pq ${path_norm_pq} -path_data ${path_data} -path_edges ${path_edges} -path_info ${path_info} -path_gt ${path_gt} -path_q ${path_q} -path_groups ${path_groups} -path_idxs ${path_idxs} -k ${k} -n ${n} -nq ${nq} -nc ${ncentroids} -M ${M} -M_PQ ${M_PQ} -efConstruction ${efConstruction} -efSearch ${efSearch} -d ${d} -gt_d ${gt_d} -nprobes ${nprobes} -max_codes ${max_codes} -nsubc ${nsubcentroids}
done

nprobes="128"
max_codes="50000"
efSearch="140"

for i in {1..5}
do
/home/dbaranchuk/ivf-hnsw/bin/demo_ivfhnsw_grouping_sift1b -path_centroids ${path_centroids} -path_learn ${path_learn} -path_index ${path_index} -path_precomputed_idx ${path_precomputed_idxs} -path_pq ${path_pq} -path_norm_pq ${path_norm_pq} -path_data ${path_data} -path_edges ${path_edges} -path_info ${path_info} -path_gt ${path_gt} -path_q ${path_q} -path_groups ${path_groups} -path_idxs ${path_idxs} -k ${k} -n ${n} -nq ${nq} -nc ${ncentroids} -M ${M} -M_PQ ${M_PQ} -efConstruction ${efConstruction} -efSearch ${efSearch} -d ${d} -gt_d ${gt_d} -nprobes ${nprobes} -max_codes ${max_codes} -nsubc ${nsubcentroids}
done
