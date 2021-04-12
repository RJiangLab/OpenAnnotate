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

knumb=0
for((i = 1; i <= 1; i++)); do
	rnumb=0
	for spath in $(find $TPATH/$FPTRN -mindepth 0 -maxdepth 0 -type d | sort); do
		sname=`basename $spath`
		dname=$sname
		tname=$sname
		kname=$sname
		shome=$spath
		phome=$PPATH
		dhome=$DPATH/$sname
		thome=$TPATH/$sname
		khome=$KPATH/$sname
		if [[ -e $khome/$kname.err && ! -s $khome/$kname.err ]]; then
			continue
		fi
		knumb=$((knumb+1))
		rnumb=$((rnumb+1))
		nodes=($(awk 'NR==FNR{nxx[$0]++;next}{if($2 in nxx){print $0"\t"nxx[$2]}else{print $0"\t0"}}' <(squeue -u $USER | awk '{print $8}') <(sinfo -N -h -o "%R/%N/%C" | sed -r 's/\//\t/g' | awk '$4>0{print $1"\t"$2}') | grep -v -P "s01" | sort -n -k3,3 | head -n 1))

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
#SBATCH -t 0-24:00                                      # Runtime in D-HH:MM
#SBATCH -p ${nodes[0]}                                  # Partition to submit to
#SBATCH -w ${nodes[1]}                                  # Node to submit to
#SBATCH -o $khome/$kname.out                            # File to which STDOUT will be written
#SBATCH -e $khome/$kname.err                            # File to which STDERR will be written
#SBATCH --mem=4000                                      # Memory pool for all cores
#SBATCH --mail-type=FAIL                                # Type of email notification- BEGIN,END,FAIL,ALL
#SBATCH --mail-user=rjiang@tsinghua.org.cn              # Email to which notifications
 source \$HOME/.bashrc
 source /prog/setup.sh
#printenv

hname=\`hostname\`
sname=$sname
dname=$dname
tname=$tname
kname=$kname
phome=$phome
shome=$shome
dhome=$dhome
thome=$thome
khome=$khome
ttemp=\`echo \$thome | sed -r 's/home/tmp/'\`
dfgrc=\`echo \$dhome | sed -r 's/read/fgrc/'\`
dbgrc=\`echo \$dhome | sed -r 's/read/bgrc/'\`

mkdir -p \$ttemp
mkdir -p \$dhome
mkdir -p \$dfgrc
mkdir -p \$dbgrc
mkdir -p \$thome
cd       \$thome

printf "[%s] Start  execute script [%s].\n" \`date +%T\` \`readlink -f \$BASH_SOURCE\`
printf "\t   hname=\$hname\n"
printf "\t   sname=\$sname\n"
printf "\t   dname=\$dname\n"
printf "\t   tname=\$tname\n"
printf "\t   kname=\$kname\n"
printf "\t   phome=\$phome\n"
printf "\t   shome=\$shome\n"
printf "\t   dhome=\$dhome\n"
printf "\t   dfgrc=\$dfgrc\n"
printf "\t   dbgrc=\$dbgrc\n"
printf "\t   thome=\$thome\n"
printf "\t   khome=\$khome\n"
printf "\t   ttemp=\$ttemp\n"
printf "\t   getwd=\$PWD\n"

#bamtools 2>&1
#bedtools 2>&1

for cpath in \$(find \$thome/?? -mindepth 0 -maxdepth 0 -type d 2>/dev/null | sort); do
	cname=\`basename \$cpath\`

	printf "[%s] Start  check md5sum for \$cname\n" \`date +%T\`
	rm -f \$ttemp/\$cname.md5
	for fpath in \$(find \$cpath/*.cnt.gz 2>/dev/null  | sort); do
		cat <(head -c 1M \$fpath) <(tail -c 1M \$fpath) | md5sum | cut -d ' ' -f 1 >> \$ttemp/\$cname.md5
	done
	cmsum=\`md5sum \$ttemp/\$cname.md5 | cut -d ' ' -f 1\`
	if [[ ! -e \$cname.md5 || \`cat \$cname.md5\` != \$cmsum ]]; then
		rm  -f \$cname.md5 2>/dev/null
	fi
	printf "[%s] Finish check md5sum for \$cname\n" \`date +%T\`

	if [[ ! -e \$cname.md5 ]]; then
		printf "[%s] Start  create \$thome/\$cname??.*.bin\n" \`date +%T\`
		rm -f \$cname.txt
		for fpath in \$(find \$cpath/*.cnt.gz 2>/dev/null | sort); do
			echo \$fpath >> \$cname.txt
		done
		if [[ -s \$cname.txt ]]; then	
			\$phome/create_fbin.exe \`readlink -e \$cname.txt\` \$ttemp/\$cname
			for ppart in \$(find \$ttemp/\$cname??.*.bin 2>/dev/null); do
				npart=\`basename \$ppart\`
				cp -au \$ttemp/\$npart \$thome/
				if [[ ! -s \$thome/\$npart ]]; then
					printf "[%s] Error. \$thome/\$npart does not exist.\n" \`date +%T\` 1>&2
					rm  -f \$thome/\$cname.txt
					rm  -f \$thome/\$cname.md5
					exit 1
				fi
			done
			echo \$cmsum > \$cname.md5
		fi
		printf "[%s] Finish create \$thome/\$cname??.*.bin\n" \`date +%T\`
	fi

	if [[ -e \$cname.md5 ]]; then
		for pfgrc in \$(find \$thome/\$cname??.fgrc.bin 2>/dev/null); do
			nfgrc=\`basename \$pfgrc | sed -r 's/.fgrc//'\`
			rm -f \$dfgrc/\$nfgrc
			ln -s \$pfgrc \$dfgrc/\$nfgrc
		done
		for pbgrc in \$(find \$thome/\$cname??.bgrc.bin 2>/dev/null); do
			nbgrc=\`basename \$pbgrc | sed -r 's/.bgrc//'\`
			rm -f \$dbgrc/\$nbgrc
			ln -s \$pbgrc \$dbgrc/\$nbgrc
		done
	fi
done

rm -rf \$ttemp
printf "[%s] Finish execute script [%s].\n" \`date +%T\` \`readlink -f \$BASH_SOURCE\`
SCRIPT
		chmod  +x $kname.sh
		sbatch    $kname.sh 1>>$khome/$kname.out 2>>$khome/$kname.err
		sleep     $((0+${nodes[2]}*${nodes[2]}*1))
#		exit
	done
	if [[ $rnumb -eq 0 ]]; then
		break
	fi
	while [ `squeue -h -u "$USER" | wc -l` -gt 0 ]; do sleep 10s; done
done
