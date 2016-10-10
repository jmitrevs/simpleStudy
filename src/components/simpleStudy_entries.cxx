#include "GaudiKernel/DeclareFactoryEntries.h"
#include "simpleStudy/TestAlg.h"

DECLARE_ALGORITHM_FACTORY( TestAlg )

DECLARE_FACTORY_ENTRIES( simpleStudy )
{
	DECLARE_ALGORITHM( TestAlg )
}
