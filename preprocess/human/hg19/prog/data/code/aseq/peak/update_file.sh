#!/bin/bash
export LC_ALL=C

PPATH=`readlink -f $BASH_SOURCE | sed -r 's/^\.\///g'`
PTEXT=(${PPATH//\// })
FNAME=${PTEXT[${#PTEXT[@]}-2]}
ENAME=${PTEXT[${#PTEXT[@]}-3]}
PNAME=${PTEXT[${#PTEXT[@]}-4]}
GNAME=${PTEXT[${#PTEXT[@]}-7]}
SNAME=${PTEXT[${#PTEXT[@]}-8]}
DNAME=${PTEXT[${#PTEXT[@]}-9]}
KNAME="file"
PPATH=`dirname $PPATH`
DPATH=`echo $PPATH | sed -r 's/prog/data/g'`/$KNAME
WPATH=`echo $PPATH | sed -r 's/prog/work/g'`/$KNAME
TPATH=`echo $PPATH | sed -r 's/prog/temp/g'`/$KNAME
KPATH=`echo $PPATH | sed -r 's/prog/task/g; s/home/temp/g'`/$KNAME

echo "DNAME=$DNAME"
echo "SNAME=$SNAME"
echo "GNAME=$GNAME"
echo "PNAME=$PNAME"
echo "ENAME=$ENAME"
echo "FNAME=$FNAME"
echo "KNAME=$KNAME"
echo "PPATH=$PPATH"
echo "DPATH=$DPATH"
echo "WPATH=$WPATH"
echo "TPATH=$TPATH"
echo "KPATH=$KPATH"
#exit

#:<<\COMMENT
rm    -rf $KPATH
mkdir -p  $TPATH
mkdir -p  $KPATH
cd        $TPATH
#COMMENT
echo "GETWD=$PWD"
#exit

FPTRN="*"
if   [[ $# -eq 0 ]]; then
	FPTRN="*"
elif [[ $# -ge 1 ]]; then
	FPTRN=$1
fi
echo "FPTRN=$FPTRN"
#exit

# declare -A GNMAP
# GNMAP=([hg19]=hg19 [hg38]=GRCh38)
# gname=${GNMAP[$GNAME]}
# hnumb=0 
# awk  -F "\t" '
# NR == FNR {
# 	bams[$1]++;
# 	next;
# }
# $3 in bams {
# 	print
# }'   <(
# zcat -f `echo $DPATH | sed -r 's/'"$ENAME"'/meta/; s/'"$FNAME"'/meta/; s/'"$KNAME"'/meta/'` |
# grep -P "$gname" | 
# grep -P "DNase-seq" | 
# grep -P "bam" | 
# grep -v -P "unfiltered" | 
# awk  -F "\t" '{print $1}' | 
# sort  |
# uniq  ) <(
# zcat -f `echo $DPATH | sed -r 's/'"$ENAME"'/meta/; s/'"$FNAME"'/meta/; s/'"$KNAME"'/meta/'` |
# grep -P "$gname" | 
# grep -P "DNase-seq" | 
# grep -P "bed narrowPeak" | 
# awk  -F "\t" '$36 != "" {print $1 "\t" $4 "\t" $36}' | 
# sed  -r 's/, /\t/g' | 
# awk  -F "\t" '{for(i=3; i <= NF; i++){print $2 "\t" $1 "\t" $i}}' | 
# sort -k 2,2 | 
# uniq  ) |
while read line; do
	items=(${line//\t/ })
	ename=${items[4]}
	pname=${items[0]}
	fname=${items[2]}
	epath=`echo $TPATH`/$ename
	ppath=`echo $TPATH`/$pname
	fpath=`echo $DPATH | sed -r 's/'"$FNAME"'/cnts/; s/'"$KNAME"'/file/'`/$fname
	dpath=/data/encode/human/aseq/peak/$GNAME/$pname/$pname.bed.gz
	echo $dpath
	if [[ ! -e $dpath ]]; then
		continue;
	fi
	if [[ -e $ppath ]]; then
		continue;
	fi
	hnumb=$((hnumb+1))
	printf "%6d\t%s\t%s\t%s\n" $hnumb $ename $ppath $fpath
	for((i = 1; i <= 25; i++)); do
		cname=`printf "%02d.cnt.gz" $i`
		dname=`printf "%02d" $i`
		if [[ -s $fpath/$cname ]]; then
			if [[ `readlink -e $fpath/$cname` != `readlink -e $ppath/$dname/$fname` ]]; then
				mkdir -p $ppath/$dname
				rm    -f $ppath/$dname/$fname.cnt.gz
				ln    -s $fpath/$cname $ppath/$dname/$fname.cnt.gz
			fi
		else
			rm -f $ppath/$dname/$fname.cnt.gz
		fi
	done

	if [[ -e $ppath && $dlast != $dpath ]]; then
		zcat -f $dpath |
		awk  -v pname=$pname -v ppath=$ppath '
		BEGIN {
        	for(i = 1; i <= 22; i++){
            	chr[sprintf("chr%d",i)] = i;
	        }
    	    chr["chrX"] = 23;
	        chr["chrY"] = 24;
    	    chr["chrM"] = 25;
        	for(k in chr){
            	file[k] = sprintf("%s/%02d/%s.bed.gz", ppath, chr[k], pname);
	        }
		}
		{
			print | "gzip -c > "file[$1]
		}'
		dlast=$dpath
	fi
done < ../../../anno/code/aseq/file/update_bins.txt
