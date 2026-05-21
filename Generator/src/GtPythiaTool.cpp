#include "GtPythiaTool.hh"

DECLARE_COMPONENT(GtPythiaTool)

StatusCode GtPythiaTool::initialize() {
    StatusCode sc;
    m_pythia = std::make_unique<Pythia8::Pythia>();
    // configure with the card file first
    m_pythia->readFile(m_card.value());
    // tune with the additional commands
    for (auto cmd: m_cmds.value()) {
        m_pythia->readString(cmd);
    }
    // initialize pythia
    m_pythia->init();
    return sc;
}

StatusCode GtPythiaTool::finalize() {
    StatusCode sc;
    return sc;
}

bool GtPythiaTool::mutate(Gen::GenEvent& event) {
    // generate the event
    while(!m_pythia->next()) {
        // if failed, try again
    }

    // get the particles
    auto& pythia_particles = m_pythia->event;
    // loop over the particles
    for (int i = 0; i < pythia_particles.size(); ++i) {
        auto& p = pythia_particles[i];
        // create the MCParticle
        auto mcp = event.getMCVec().create();
        // set the properties
        mcp.setPDG(p.id());
        int status = 0;
        if (p.isFinal()) {
            status = 1;
        } else {
            status = 0;
        }
        mcp.setGeneratorStatus(status);
        mcp.setCharge(p.charge());
        mcp.setTime(p.tProd());
        mcp.setMass(p.m());
        mcp.setVertex(edm4hep::Vector3d(p.xProd(), p.yProd(), p.zProd()));
        // update the endpoint later
        mcp.setEndpoint(edm4hep::Vector3d(p.xProd(), p.yProd(), p.zProd()));
        mcp.setMomentum(edm4hep::Vector3f(p.px(), p.py(), p.pz()));
    }
    // setup the relationships (mother and daughter)
    for (int i = 0; i < pythia_particles.size(); ++i) {
        auto& p = pythia_particles[i];
        auto mcp = event.getMCVec()[i];

        auto mother_list = p.motherList();
        for (auto idx: mother_list) {
            auto mother = event.getMCVec()[idx];
            mcp.addToParents(mother);
        }

        auto daughter_list = p.daughterList();
        bool need_endpoint = true;
        for (auto idx: daughter_list) {
            auto daughter = event.getMCVec()[idx];
            mcp.addToDaughters(daughter);
            // Update the endpoint to the daughter's vertex
            if (need_endpoint) {
                mcp.setEndpoint(daughter.getVertex());
                need_endpoint = false;
            }
        }
    }
    return true;
}

bool GtPythiaTool::finish() {
    return true;
}

bool GtPythiaTool::configure_gentool() {
    return true;
}