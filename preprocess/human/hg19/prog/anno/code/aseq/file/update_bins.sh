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
KNAME="bins"
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

declare -A GNMAP
GNMAP=([hg19]=hg19 [hg38]=GRCh38)
gname=${GNMAP[$GNAME]}
hpath=`dirname $TPATH`/head/head
mkdir -p `dirname $hpath`
#exit

zcat -f $PPATH/update_bins.txt |
awk -F "\t" '
($1!="")&&($2!=""){
	for(i=1; i<=NF; i++){
		gsub(/^ *| *$/,"", $i)
	}
	printf("%s", $1)
	for(i=2; i<=NF; i++){
		printf("\t%s", $i);
	}
	printf("\n");
}' | 
#sort -t $'\t' -s -k1,1   |
#sort -t $'\t' -s -k2,2   |
sort -t $'\t' -s -k3,3   |
#sort -t $'\t' -s -k4,4   |
sort -t $'\t' -s -k5,5   |
sort -t $'\t' -s -k6,6   |
sort -t $'\t' -s -k7,7   |
sort -t $'\t' -s -k8,8   |
sort -t $'\t' -s -k9,9   |
sort -t $'\t' -s -k10,10 |
sort -t $'\t' -s -k11,11 |
cat   > $hpath.all

cat $hpath.all |
awk -F "\t" '
{
	print $3 "\t" $4 "\t" $5 "\tATAC\t" $6 "\t" $7 "\t" $9 "\t" $11
}' |
gzip -c > $hpath

cat $hpath.all |
awk -F "\t" '
{
	print $11 "\t" $9 "\t" $7 "\t" $6 "\tATAC\t" $5 "\t" $4 "\t" $3
}' |
gzip -c > $hpath.row

cat $hpath.all | cut -f 11 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "%s",$1} END{printf "\n"}' | gzip -c >  $hpath.col
cat $hpath.all | cut -f  9 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "%s",$1} END{printf "\n"}' | gzip -c >> $hpath.col
cat $hpath.all | cut -f  7 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "%s",$1} END{printf "\n"}' | gzip -c >> $hpath.col
cat $hpath.all | cut -f  6 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "%s",$1} END{printf "\n"}' | gzip -c >> $hpath.col
cat $hpath.all | cut -f  6 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "ATAC"  } END{printf "\n"}'| gzip -c >> $hpath.col
cat $hpath.all | cut -f  5 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "%s",$1} END{printf "\n"}' | gzip -c >> $hpath.col
cat $hpath.all | cut -f  4 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "%s",$1} END{printf "\n"}' | gzip -c >> $hpath.col
cat $hpath.all | cut -f  3 | awk -F "\t" 'BEGIN{printf "#";} NR>1{printf "\t"} {printf "%s",$1} END{printf "\n"}' | gzip -c >> $hpath.col

# exit

aname=("peak" "spot" "fgrc" "bgrc")
aindx=(0 1 2 2)
zcat -f $PPATH/update_bins.txt |
awk -F "\t" '($1!="")&&($2!=""){print}' | 
while read line; do
	items=($line)
	for((i = 0; i < ${#aname[*]}; i++)); do
		rname=${items[2]}
		index=${aindx[$i]}
		iname=${aname[$i]}
		dname=${items[$index]}
		dpath=$(echo $DPATH | sed -r 's/anno/data/; s/file/'"$iname"'/; s/bins/file/')/$dname
		echo $iname $rname $dpath
		if [ ! -e $dpath ]; then
			echo "Error. $dpath does not exist."
			exit 1
		fi
		for fpath in $(find $dpath/* -mindepth 0 -maxdepth 0 ! -type d | sort | uniq); do
			cname=`basename $fpath | sed -r 's/.bin//g'`
			ppath=$TPATH/$cname/$iname/$rname
			if [ [`readlink -e $ppath` == `readlink -f $fpath`] ]; then
				continue
			fi
			echo $iname $rname $fpath $ppath
#continue
			if [ ! -e `dirname $ppath` ]; then
				mkdir -p `dirname $ppath`
			fi
			rm -f $ppath
			ln -s $fpath $ppath
		done
	done
done
