#include "GaudiKernel/DeclareFactoryEntries.h"
#include "simpleStudy/TestAlg.h"
#include "simpleStudy/TruthUtils.h"

DECLARE_ALGORITHM_FACTORY( TestAlg )
DECLARE_TOOL_FACTORY( TruthUtils )

DECLARE_FACTORY_ENTRIES( simpleStudy )
{
	DECLARE_ALGORITHM( TestAlg )
	  DECLARE_TOOL( TruthUtils )
}
