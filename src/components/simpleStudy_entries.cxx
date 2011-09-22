#include "GaudiKernel/DeclareFactoryEntries.h"
#include "InsituAnalysis/InsituAnalysis_Selection.h"
#include "InsituAnalysis/InsituAnalysis_ElSelection.h"
#include "InsituAnalysis/InsituAnalysis_Performance.h"
#include "InsituAnalysis/TestAlg.h"
#include "InsituAnalysis/TestAlgESD.h"
#include "InsituAnalysis/TruthUtils.h"

DECLARE_ALGORITHM_FACTORY( InsituAnalysis_Selection )
DECLARE_ALGORITHM_FACTORY( InsituAnalysis_ElSelection )
DECLARE_ALGORITHM_FACTORY( InsituAnalysis_Performance )
DECLARE_ALGORITHM_FACTORY( TestAlg )
DECLARE_ALGORITHM_FACTORY( TestAlgESD )
DECLARE_TOOL_FACTORY( TruthUtils )

DECLARE_FACTORY_ENTRIES( InsituAnalysis )
{
	DECLARE_ALGORITHM( InsituAnalysis_Selection )
	DECLARE_ALGORITHM( InsituAnalysis_ElSelection )
	DECLARE_ALGORITHM( InsituAnalysis_Performance )
	DECLARE_ALGORITHM( TestAlg )
	DECLARE_ALGORITHM( TestAlgESD )
	  DECLARE_TOOL( TruthUtils )
}
