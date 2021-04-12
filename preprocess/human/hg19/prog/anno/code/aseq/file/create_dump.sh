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
KNAME="dump"
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
echo "gname=$gname"
#exit

knumb=0
for((i = 1; i <= 1; i++)); do
	rnumb=0
	for spath in $(find `echo $TPATH | sed -r 's/dump/bins/g'`/$FPTRN -mindepth 0 -maxdepth 0 -type d); do
		sname=`basename $spath`
		dname=$sname
		tname=$sname
		kname=$sname
		rname=s01:
		ehome=$PPATH
		shome=$spath
		dhome=$DPATH/$sname
		thome=$TPATH/$sname
		khome=$KPATH/$sname
		if [[ -e $khome/$kname.err && ! -s $khome/$kname.err ]]; then
			continue
		fi
		knumb=$((knumb+1))
		rnumb=$((rnumb+1))
		nodes=($(awk 'NR==FNR{nxx[$0]++;next}{if($2 in nxx){print $0"\t"nxx[$2]}else{print $0"\t0"}}' <(squeue -u $USER | awk '{print $8}') <(sinfo -N -h -o "%R/%N/%C" | sed -r 's/\//\t/g' | awk '$4>0{print $1"\t"$2}') | grep -v -P "s03" | sort -s -n -k3,3 | head -n 1))

		printf "[%s] %06d\t%06d\t%s\t%s\t%s\t%s\t%s\n" `date +%T` $knumb $rnumb ${nodes[1]} $kname $shome $dhome
#		continue

		mkdir -p $khome
		cd       $khome
		rm    -f $kname.out
		rm    -f $kname.err
		rm    -f $kname.sh
		cat    > $kname.sh <<SCRIPT
#!/bin/bash
#SBATCH -n 1                                            # Number of cores
#SBATCH -N 1                                            # Ensure that all cores are on one machine
#SBATCH -t 3-00:00                                      # Runtime in D-HH:MM
#SBATCH -p ${nodes[0]}                                  # Partition to submit to
#SBATCH -w ${nodes[1]}                                  # Node to submit to
#SBATCH -o $khome/$kname.out                            # File to which STDOUT will be written
#SBATCH -e $khome/$kname.err                            # File to which STDERR will be written
#SBATCH --mem=8000                                      # Memory pool for all cores
#SBATCH --mail-type=FAIL                                # Type of email notification- BEGIN,END,FAIL,ALL
 source \$HOME/.bashrc
 source /prog/setup.sh
#printenv
#SBATCH --mail-user=rjiang@tsinghua.org.cn              # Email to which notifications

hname=\`hostname\`
sname=$sname
dname=$dname
tname=$tname
kname=$kname
rname=$rname
ehome=$ehome
shome=$shome
dhome=$dhome
thome=$thome
khome=$khome
ttemp=\`echo \$thome | sed -r 's/home/tmp/'\`
sbins=\$shome
shead=\$(readlink -e \$sbins/../..)/head
tbins=\$ttemp/bins
thead=\$ttemp/head

mkdir -p \$ttemp
mkdir -p \$tbins
mkdir -p \$thead
cd       \$ttemp

printf "[%s] Start  execute script [%s].\n" \`date +%T\` \`readlink -f \$BASH_SOURCE\`
printf "\t   hname=\$hname\n"
printf "\t   sname=\$sname\n"
printf "\t   dname=\$dname\n"
printf "\t   tname=\$tname\n"
printf "\t   kname=\$kname\n"
printf "\t   ehome=\$ehome\n"
printf "\t   shome=\$shome\n"
printf "\t   dhome=\$dhome\n"
printf "\t   thome=\$thome\n"
printf "\t   khome=\$khome\n"
printf "\t   sbins=\$sbins\n"
printf "\t   shead=\$shead\n"
printf "\t   tbins= \$tbins\n"
printf "\t   thead= \$thead\n"
printf "\t   ttemp= \$ttemp\n"
printf "\t   getwd= \$PWD\n"
#exit

#:<<\\COMMENT
printf "[%s] Start  copy data files, from     [%s].\n" \`date +%T\` \$rname\$sbins
for((j = 0; j < 3; j++)); do
	rsync -aL --delete \$rname\$sbins/ \$tbins/
done
printf "[%s] Finish copy data files, save to  [%s].\n" \`date +%T\` \$tbins
#COMMENT


#:<<\\COMMENT
printf "[%s] Start  consistency check for     [%s].\n" \`date +%T\` \$tbins
for sfile in \$(find \$sbins/* ! -type d); do
	tfile=\`echo \$sfile | sed -r 's|'"\$sbins"'|'"\$tbins"'|'\`
	sreal=\`readlink -e \$sfile\`
	treal=\`readlink -e \$tfile\`
	if [[ -e \$sreal ]]; then
		stime=\`stat -c "%Y" \$sreal\`
		ssize=\`stat -c "%s" \$sreal\`
		smsum=\`cat <(head -c 1K \$sreal) <(tail -c 1K \$sreal) | md5sum | cut -d ' ' -f 1\`
	else
		echo "\$sreal does not exist." >&2 
		exit 1
	fi
	if [[ -e \$treal ]]; then
		ttime=\`stat -c "%Y" \$treal\`
		tsize=\`stat -c "%s" \$treal\`
		tmsum=\`cat <(head -c 1K \$treal) <(tail -c 1K \$treal) | md5sum | cut -d ' ' -f 1\`
	else
		echo "\$treal does not exist." >&2 
		exit 1
	fi
	if [[ \$stime -ne \$ttime ]]; then
		echo "File time does not match (\$stime != \$ttime) for \$tfile." >&2 
		exit 1
	fi
	if [[ \$ssize -ne \$tsize ]]; then
		echo "File size does not match (\$ssize != \$tsize) for \$tfile." >&2 
		exit 1
	fi
	if [[ \$smsum != \$tmsum ]]; then
		echo "File msum does not match (\$smsum != \$tmsum) for \$tfile." >&2 
		exit 1
	fi
done
printf "[%s] Finish consistency check for     [%s].\n" \`date +%T\` \$tbins
#COMMENT


#:<<\\COMMENT
printf "[%s] Start  create head file, save to [%s].\n" \`date +%T\` \$thead
rm -f \$thead/\$kname
aname=("fgrc" "bgrc" "peak" "spot")
for iname in \${aname[*]}; do
	rm -f \$thead/\$iname
	zcat -f \$shead/head | cut -f 1 | while read ename; do
		echo  \$tbins/\$iname/\$ename >> \$thead/\$iname
	done
#	for epath in \$(find \$tbins/\$iname/* ! -type d | sort | uniq); do
#		echo \`readlink -e \$epath\` >> \$thead/\$iname
#	done
	echo \`readlink -e \$thead/\$iname\` >> \$thead/\$kname
done
printf "[%s] Finish create head file, save to [%s].\n" \`date +%T\` \$thead

pexec=\$ehome/create_xlib.exe
#axlib=("lz4z" "zlib" "bz2z" "lzma")
axlib=("lz4z")
for((i = 0; i < \${#axlib[*]}; i++)); do
	nxlib=\${axlib[\$i]}
	txlib=\`echo \$tbins | sed -r 's/bins/'"\$nxlib"'/'\`
	hxlib=\`echo \$thome | sed -r 's/dump/'"\$nxlib"'/'\`
	dxlib=\`echo \$hxlib | sed -r 's/temp/data/'\`

	printf "[%s] Start  create %s dump, save to [%s].\n" \`date +%T\` \$nxlib \$txlib
	printf "\t   pexec=\$pexec\n"
	printf "\t   dxlib=\$dxlib\n"
	printf "\t   hxlib=\$hxlib\n"
	printf "\t   txlib= \$txlib\n"
	printf "\t   thead= \$thead\n"

	rm    -rf \$txlib
	mkdir -p  \$txlib
	mkdir -p  \$hxlib

	touch \$khome/\$nxlib.run
	\`
    \$pexec \$nxlib \$thead/\$kname \$txlib/\$kname 1>>\$khome/\$kname.out 2>>\$khome/\$kname.err

	sreal=\\\`readlink -f   \$txlib/\$kname.bin\\\`
	ftime=\\\`stat -c "%Y"  \$sreal\\\`
	fsize=\\\`stat -c "%s"  \$sreal\\\`
	fmsum=\\\`md5sum        \$sreal | cut -d ' ' -f 1\\\`
	echo \$fmsum > \$txlib/\$kname.md5
	for((j = 0; j < 3; j++)); do
		rsync -aL --delete \$txlib/ \$rname\$hxlib/
	done
	if [ -s \$hxlib/\$kname.bin ]; then
		mkdir -p \\\`dirname \$dxlib\\\`
		rm    -f \$dxlib
		ln    -s \$hxlib/\$kname.bin \$dxlib
	else
		printf "File does not exist [\$hxlib/\$kname.bin].\n" >> \$khome/\$kname.err
	fi
	rm -rf \$txlib
	rm -f  \$khome/\$nxlib.run
	printf "[%s] Finish create %s dump, copy to [%s].\n" \\\`date +%T\\\` \$nxlib \$rname\$hxlib >> \$khome/\$kname.out
	\` &
done
#COMMENT

sleep 5s; while [ \`find \$khome/*.run 2>/dev/null | wc -l\` -gt 0 ]; do sleep 5s; done

rm -rf \$ttemp
printf "[%s] Finish execute script [%s].\n" \`date +%T\` \`readlink -f \$BASH_SOURCE\`
SCRIPT
		chmod  +x $kname.sh
		sbatch    $kname.sh 1>>$khome/$kname.out 2>>$khome/$kname.err
		sleep     $((1+${nodes[2]}*${nodes[2]}*1))
#		exit
	done
	if [[ $rnumb -eq 0 ]]; then
		break
	fi
	while [ `squeue -h -u "$USER" | wc -l` -gt 0 ]; do sleep 10s; done
done
