#include <set>
#include <vector>
#include <string.h>
#include "eutil.h"
#include "tpipe.h"
#include "tannoload.h"
#include "tannosave.h"
#include "tannowork.h"
#include "tannolocal.h"


BEGIN_OPTION_TABLE(TAnnoLocalOpts)
	OPTION_ENTRY(annotate,		 ,	NONARG,	TRUE)
	OPTION_ENTRY(species,		s,	REQARG,	human)
	OPTION_ENTRY(assembly,		a,	REQARG,	hg19)
	OPTION_ENTRY(assay,			y,	REQARG,	dnase)
	OPTION_ENTRY(column,	 	c,	REQARG,	NULL)
	OPTION_ENTRY(foreground,	f,	REQARG,	NULL)
	OPTION_ENTRY(background,	g,	REQARG,	NULL)
	OPTION_ENTRY(narrowpeak,	n,	REQARG,	NULL)
	OPTION_ENTRY(broadpeak,		b,	REQARG,	NULL)
	OPTION_ENTRY(readopen,		r,	REQARG,	NULL)
	OPTION_ENTRY(binary-path,	p,	REQARG,	NULL)
	OPTION_ENTRY(head-file,		e,	REQARG,	NULL)
	OPTION_ENTRY(input-file,	i,	REQARG, NULL)
	OPTION_ENTRY(option-file,	 ,	REQARG,	NULL)
	OPTION_ENTRY(status-file,	 ,	REQARG,	NULL)
	OPTION_ENTRY(perbasepair, 	 ,	NONARG,	FALSE)
	OPTION_ENTRY(system,	 	 ,	REQARG,	NULL)
	OPTION_ENTRY(target,	 	 ,	REQARG,	NULL)
	OPTION_ENTRY(organ,	 	 	 ,	REQARG,	NULL)
	OPTION_ENTRY(celltype, 	 	 ,	REQARG,	NULL)
	OPTION_ENTRY(experiment, 	 ,	REQARG,	NULL)
	OPTION_ENTRY(biosample, 	 ,	REQARG,	NULL)
	OPTION_ENTRY(replicate, 	 ,	REQARG,	NULL)
END_OPTION_TABLE(TAnnoOpts)


int TAnnoLocal::LoadOptions(int argc, char * argv[]) 
{
	if((opts=new TAnnoLocalOpts) != NULL) {
		opts->LoadOptions(argc, argv);
	}
#if 0
	{
		FILE * fopt = fopen("/tmp/openanno.arg", "wb");
		opts->SaveOptions(fopt);
		fclose(fopt);
	}
#endif
#if 0
	{
		FILE * fopt = fopen("/tmp/openanno.arg", "rb");
		opts->LoadOptions(fopt);
		fclose(fopt);
	}
#endif
#if 0
	((TOpts*)opts)->DumpOptions();
//	exit(1);
#endif
	return opts ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TAnnoLocal::Main() 
{
	int nret = EXIT_SUCCESS;
	if( strcasecmp((*opts)["option-file"], "NULL")) {
		nret = OnLoadIni();
	}
	if( strcasecmp((*opts)["help"], "TRUE") == 0) {
		nret = OnHelp();
	} else if( strcasecmp((*opts)["annotate"], "TRUE") == 0) {
		if((nret = CheckArgs()) == EXIT_SUCCESS) {
			nret = OnAnnotate();
		}
	}
	if( nret != EXIT_SUCCESS) {
		nret  = OnHelp();
	}
	return nret;
}

int TAnnoLocal::OnHelp()
{
	fprintf(stdout, "Usage: openanno --species=human --assembly=hg19 --assay=dnase --foreground=foreground.txt.gz --input-file=somefile.bed.gz\n");
	return EXIT_SUCCESS;
}

int TAnnoLocal::OnLoadIni()
{
	fprintf(stdout, "Load ini file [%s].\n", (*opts)["option-file"]);
	return EXIT_SUCCESS;
}

int TAnnoLocal::OnAnnotate() 
{
	vector<IAnno*> anno;
	vector<void *> args;
	vector<IPipe*> pipe;

	anno.push_back(new TAnnoLoad);
	anno.push_back(new TAnnoSave);
	for(int i = 0, I = atoi((*opts)["thread"]); i < I; i++) {
		anno.push_back(new TAnnoWork);
	}
	for(int i = 0; i < anno.size(); i++) {
		args.push_back(NULL);
	}

	pipe.push_back(new TQueuePipe(  128));
	pipe.push_back(new TQueuePipe(32768));
	pipe.push_back(new TQueuePipe(  128));

	anno[0]->Attach(pipe[0], PIPE_OUT);
	anno[1]->Attach(pipe[1], PIPE_IN);
	for(int i = 2; i < anno.size(); i++) {
		anno[i]->Attach(pipe[0], PIPE_IN);
		anno[i]->Attach(pipe[1], PIPE_OUT);
	}
//	anno[0]->Attach(pipe[2], PIPE_IN);
//	anno[1]->Attach(pipe[2], PIPE_OUT);

    for(int i = 0, I = anno.size(); i < I; i++) {
    	anno[i]->Run(opts, args[i]);
    }
    for(int i = 0, I = anno.size(); i < I; i++) {
        anno[i]->Wait(NULL);
    }
    for(int i = 0, I = anno.size(); i < I; i++) {
        delete anno[i];
    }
    for(int i = 0, I = pipe.size(); i < I; i++) {
        delete pipe[i];
    }

	return EXIT_SUCCESS;
}

int TAnnoLocal::CheckArgs()
{
	int nret = EXIT_SUCCESS;
	if(strcasecmp((*opts)["input-file"], "NULL") == 0) {
		nret = EXIT_FAILURE;
//	} else if( strcasecmp((*opts)["species"], "human")) {
//		nret = EXIT_FAILURE;
//	} else if(strcasecmp((*opts)["assembly"], "hg19") && strcasecmp((*opts)["assembly"], "hg38")) {
//		nret = EXIT_FAILURE;
	}
	return nret;
}

int main(int argc, char * argv[])
{
    int nret = EXIT_FAILURE;
    try {
		nret = AnnoMain<TAnnoLocal>(argc, argv);
    } catch(exception & e) {
        fprintf(stderr, "Failed! Exception: %s.\n", e.what());
	} 
	return nret;
}
