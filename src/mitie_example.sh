#!/bin/bash
#

#fn_bam_brain=/fml/ag-raetsch/nobackup2/projects/mip_spladder/alignments/human/ILM_S.Brain.mmr.sorted.bam
#fn_bam_uhr=/fml/ag-raetsch/nobackup2/projects/mip_spladder/alignments/human/ILM_S.UHR.mmr.sorted.bam
#fn_bam_brain_all=/fml/ag-raetsch/nobackup2/projects/mip_spladder/alignments/human/ILM_S.Brain.sorted.bam
#fn_bam_uhr_all=/fml/ag-raetsch/nobackup2/projects/mip_spladder/alignments/human/ILM_S.UHR.sorted.bam

#out_dir=/fml/ag-raetsch/nobackup/projects/mip/chris_mason/MiTie

fn_bam="testdata/*toy.bam"

out_dir=testdata/results
mkdir -p $out_dir
fn_regions=$out_dir/regions.flat

dir=`dirname $0`

if [ ! -f $fn_regions ]
then
	echo run define_regions => $fn_regions
	echo $dir/define_regions $fn_bam_few -o $fn_regions --shrink --cut-regions --max-len 100000
	$dir/define_regions $fn_bam -o $fn_regions --shrink --cut-regions --max-len 100000
fi

# create segment graph and store in file
fn_graph=${out_dir}/graphs.bin
opts="--few-regions --seg-filter 0.05 --region-filter 100 --tss-tts-pval 0.0001"

echo
echo generate graph on bam file $fn_bam
echo

$dir/generate_segment_graph ${fn_graph}.tmp $opts --regions $fn_regions $fn_bam
mv ${fn_graph}.tmp $fn_graph


##############################	
##############################	
mip_dir=$out_dir/MiTie_pred/
mkdir -p $mip_dir
MAT="/fml/ag-raetsch/share/software/matlab-7.6/bin/matlab -nojvm -nodesktop -nosplash"
addpaths="addpath matlab; "
${MAT} -r "dbstop error; $addpaths mip_paths; denovo('$fn_graph', {'`echo $fn_bam | sed "s/ /','/g"`'}, '$mip_dir'); exit"


# collect predictions and write gtf file
${MAT} -r "dbstop error; $addpaths mip_paths; collect_results('$mip_dir'); exit"

echo you can find the resulting gene prediction in $mip_dir/res_genes.mat
