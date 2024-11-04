#ifndef GenReader_h
#define GenReader_h 1

/*
 * GenReader is a base class for the gentools which load events from file
 */

#include "GenEvent.h"
#include "IGenTool.h"

class GenReader: virtual public IGenTool{

    public:
        virtual ~GenReader() = 0;
        virtual bool configure_gentool()=0;               
        virtual bool mutate(MyHepMC::GenEvent& event)=0;    
        virtual bool finish()=0;
        virtual bool isEnd()=0;

        // The start index is used to skip the first n events.
        // if start index is 0, then the first event is the first event in the file
        virtual int startIndex() { return 0; };
};

#endif
