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

declare -A GNMAP
GNMAP=([hg19]=hg19 [hg38]=GRCh38)
gname=${GNMAP[$GNAME]}
fnumb=0
for fpath in $(find `echo $DPATH | sed -r 's/read/cnts/'`/$FPTRN -mindepth 0 -maxdepth 0 -type d | sort); do
	fname=$(basename $fpath)
	fnumb=$((fnumb+1))
	bpath=`echo $TPATH`/$fname
	cpath=`echo $DPATH | sed -r 's/'"$FNAME"'/cnts/; s/'"$KNAME"'/file/'`/$fname
	printf "%4d\t%s\t%s\n" $fnumb $cpath $bpath
	for((i = 1; i <= 25; i++)); do
		cname=`printf "%02d.cnt.gz" $i`
		dname=`printf "%02d" $i`
		if [ -s $cpath/$cname ]; then
			if [[ `readlink -e $cpath/$cname` != `readlink -e $bpath/$dname/$fname` ]]; then
				mkdir -p $bpath/$dname
				rm    -f $bpath/$dname/$fname.cnt.gz
				ln    -s $cpath/$cname $bpath/$dname/$fname.cnt.gz
			fi
		else
			rm -f $bpath/$dname/$fname.cnt.gz
		fi
	done
done
