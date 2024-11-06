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
        mcp.setTime(p.tau());
        mcp.setMass(p.m());
        mcp.setVertex(edm4hep::Vector3d(p.xProd(), p.yProd(), p.zProd()));
        mcp.setEndpoint(edm4hep::Vector3d(p.xDec(), p.yDec(), p.zDec()));
        mcp.setMomentum(edm4hep::Vector3f(p.px(), p.py(), p.pz()));
    }
    return true;
}

bool GtPythiaTool::finish() {
    return true;
}

bool GtPythiaTool::configure_gentool() {
    return true;
}