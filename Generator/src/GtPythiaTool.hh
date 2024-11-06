#ifndef GtPythiaTool_hh
#define GtPythiaTool_hh

/*
 * Description:
 *   This tool is used to generate particles using Pythia.
 *   User need to specify the card.
 */

#include <GaudiKernel/AlgTool.h>
#include <Gaudi/Property.h>
#include "IGenTool.h"

#include "Pythia8/Pythia.h"

#include <vector>
#include <memory>

class GtPythiaTool: public extends<AlgTool, IGenTool> {
public:
    using extends::extends;

    // Overriding initialize and finalize
    StatusCode initialize() override;
    StatusCode finalize() override;

    // IGenTool
    bool mutate(Gen::GenEvent& event) override;
    bool finish() override;
    bool configure_gentool() override;

private:
    std::unique_ptr<Pythia8::Pythia> m_pythia;

    // below properties will be used by Pythia
    Gaudi::Property<std::vector<std::string>> m_cmds{this, "Commands"};
    Gaudi::Property<std::string> m_card{this, "Card"};
};

#endif