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
	for spath in $(find `echo $DPATH | sed -r 's/cnts/beds/'`/$FPTRN -mindepth 0 -maxdepth 0 -type d | sort); do
		sname=`basename $spath`
		sbeds=$spath/$sname.bed.gz
		dname=$sname
		tname=$sname
		kname=$sname
		shome=$spath
		dhome=$DPATH/$sname
		thome=$TPATH/$sname
		khome=$KPATH/$sname
		if [[ ! -e $sbeds ]]; then
			continue
		fi
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
shome=$shome
dhome=$dhome
thome=$thome
khome=$khome
ttemp=\`echo \$thome | sed -r 's/home/tmp/'\`

mkdir -p \$ttemp
mkdir -p \$thome
cd       \$thome

printf "[%s] Start  execute script [%s].\n" \`date +%T\` \`readlink -f \$BASH_SOURCE\`
printf "\t   hname=\$hname\n"
printf "\t   sname=\$sname\n"
printf "\t   dname=\$dname\n"
printf "\t   tname=\$tname\n"
printf "\t   kname=\$kname\n"
printf "\t   shome=\$shome\n"
printf "\t   dhome=\$dhome\n"
printf "\t   thome=\$thome\n"
printf "\t   khome=\$khome\n"
printf "\t   ttemp=\$ttemp\n"
printf "\t   getwd=\$PWD\n"

#bamtools 2>&1
#bedtools 2>&1
#curl -s -L "http://hgdownload.soe.ucsc.edu/goldenPath/hg19/database/chromInfo.txt.gz" | zcat -f - | grep -P "chr\S{1,2}\s+"

cat <(head -c 1M \$shome/\$sname.bed.gz) <(tail -c 1M  \$shome/\$tname.bed.gz) | md5sum | cut -d ' ' -f 1 > \$ttemp/\$tname.md5
if [[ ! -e \$tname.md5 || \`cat \$tname.md5\` != \`cat \$ttemp/\$tname.md5\` ]]; then
	printf "[%s] Start  link \$shome/\$sname.bed.gz to \$thome/\$tname.bed.gz\n" \`date +%T\`
	rm  -f \$thome/*
	ln  -s \$shome/\$sname.bed.gz  \$tname.bed.gz
	cp  -a \$ttemp/\$tname.md5 \$thome/
	printf "[%s] Finish link \$shome/\$sname.bed.gz to \$thome/\$tname.bed.gz\n" \`date +%T\`
fi
if [[ ! -e \$tname.cnt.gz || \`stat -c "%s" \$tname.cnt.gz\` -lt 1000000 ]]; then
	printf "[%s] Start  create \$thome/\$tname.cnt.gz\n" \`date +%T\`
	rm   -f \$tname.cnt.gz
	curl -s -L "http://hgdownload.soe.ucsc.edu/goldenPath/hg19/database/chromInfo.txt.gz" | zcat -f - | grep -P "chr\S{1,2}\s+" |
	awk '
	BEGIN {
        for(i = 1; i <= 22; i++){
            chr[sprintf("chr%d",i)] = i;
        }
        chr["chrX"] = 23;
        chr["chrY"] = 24;
        chr["chrM"] = 25;
	}
	{
		printf("%02d\t%s\t%d\n", chr[\$1], \$1, \$2);
	}' | sort -k 1,1 > \$ttemp/cinfo
	zcat -f \$tname.bed.gz |
	awk  -F "\t" -v ttemp=\$ttemp '
	BEGIN {
		while("cat "ttemp"/cinfo" | getline){
        	chr[\$2] = \$1;
		}
	}
	\$1 in chr {
		file=sprintf("%s/%s.001", ttemp, chr[\$1]);
		if(\$NF == "+"){
			printf("%s:%09d\n", chr[\$1], \$2)   > file;
		} else {
			printf("%s:%09d\n", chr[\$1], \$3-1) > file;
		}
	}'
	cat \$ttemp/cinfo |
	while read line; do
		item=(\${line//\\t/ })
		schr=\$ttemp/\${item[0]}.001
		tchr=\$ttemp/\${item[0]}.002
		tcnt=\$ttemp/\${item[0]}.cnt.gz
		if [[ -s \$schr ]]; then
			printf "#%s\t%d\n" \${item[1]} \${item[2]} > \$tchr
			sort \$schr |
			awk -F "\t" '
			{
				if(\$1 == last) {
					numb++;
				} else {
					if(NR > 1) {
						print last "\t" numb;
					}
					last = \$1;
					numb = 1;
				}
			}
			END {
				print last "\t" numb;
			}' >>   \$tchr
			gzip -c \$tchr  > \$tcnt
			cp   -a \$tcnt    \$thome/
			cat     \$tcnt >> \$thome/\$tname.cnt.gz
		fi
	done
	printf "[%s] Finish create \$thome/\$tname.cnt.gz\n" \`date +%T\`
fi
if [[ ! -e \$tname.cnt.gz || \`stat -c "%s" \$tname.cnt.gz\` -lt 1000000 ]]; then
	printf "[%s] Error. \$thome/\$tname.cnt.gz does not exist." \`date +%T\` 1>&2
	rm -f *.cnt.gz
	exit 1
fi
for cpath in \$(find \$thome/??.cnt.gz); do
	cname=\`basename \$cpath\`
	if [[ ! -e \$dname/\$cname || \`readlink -e \$dhome/\$cname\` !=  \`readlink -e \$thome/\$cname\` ]]; then
		printf "[%s] Start  link \$thome/\$cname to \$dhome/\$cname\n" \`date +%T\`
		mkdir -p \$dhome
		rm    -f \$dhome/\$cname
		ln    -s \$thome/\$cname \$dhome/\$cname
		printf "[%s] Finish link \$thome/\$cname to \$dhome/\$cname\n" \`date +%T\`
	fi
done

rm -rf \$ttemp
printf "[%s] Finish execute script [%s].\n" \`date +%T\` \`readlink -f \$BASH_SOURCE\`
SCRIPT
		chmod  +x $kname.sh
		sbatch    $kname.sh 1>>$khome/$kname.out 2>>$khome/$kname.err
		sleep     $((1+${nodes[2]}*${nodes[2]}*5))
#		exit
	done
	if [[ $rnumb -eq 0 ]]; then
		break
	fi
	while [ `squeue -h -u "$USER" | wc -l` -gt 0 ]; do sleep 10s; done
done
