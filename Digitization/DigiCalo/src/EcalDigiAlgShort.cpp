// Units by default: mm, ns.
// NOTE: This digitisation highly matches detector geometry CRDEcal_Short.
// TODO: read geometry information automatically.  
#include "EcalDigiAlgShort.h"

using namespace std;
using namespace dd4hep;

DECLARE_COMPONENT( EcalDigiAlgShort )

EcalDigiAlgShort::EcalDigiAlgShort(const string& name, ISvcLocator* svcLoc)
    : Algorithm(name, svcLoc), _nEvt(0)
{
    // Input collections
    declareProperty("SimCaloHitCollection", r_SimCaloCol, "Handle of the Input SimCaloHit collection");

    // Output collections
    declareProperty("CaloHitCollection", w_DigiCaloCol, "Handle of Digi CaloHit collection");
    declareProperty("CaloAssociationCollection", w_CaloAssociationCol, "Handle of CaloAssociation collection");
    declareProperty("CaloMCPAssociationCollection", w_MCPCaloAssociationCol, "Handle of CaloAssociation collection");

    algname = name;
    transform(algname.begin(), algname.end(), algname.begin(), ::tolower);
}

StatusCode EcalDigiAlgShort::initialize()
{
    if (algname.find("barrel") != string::npos)
        dettype = 1;
    else if (algname.find("endcap") != string::npos)
        dettype = 2;
    else
        throw "Incorrect detector type! Make sure that the name of the instance contains 'barrel' or 'endcap'!";

    if (_writeNtuple)
    {
        string s_outfile = _filename;
        m_wfile = new TFile(s_outfile.c_str(), "recreate");
        t_SimCont = new TTree("SimStep", "SimStep");
        t_SimHit = new TTree("SimHit", "SimHit");
        t_SimCont->Branch("step_x", &m_step_x);
        t_SimCont->Branch("step_y", &m_step_y);
        t_SimCont->Branch("step_z", &m_step_z);
        t_SimCont->Branch("step_t", &m_step_t);
        t_SimCont->Branch("stepHit_x", &m_stepHit_x);
        t_SimCont->Branch("stepHit_y", &m_stepHit_y);
        t_SimCont->Branch("stepHit_z", &m_stepHit_z);
        t_SimCont->Branch("step_E", &m_step_E);
        t_SimCont->Branch("step_T", &m_step_T);
        t_SimHit->Branch("totE_Truth", &totE_Truth);
        t_SimHit->Branch("totE_Digi", &totE_Digi);
        t_SimHit->Branch("simHit_x", &m_simHit_x);
        t_SimHit->Branch("simHit_y", &m_simHit_y);
        t_SimHit->Branch("simHit_z", &m_simHit_z);
        t_SimHit->Branch("simHit_T", &m_simHit_T);
        t_SimHit->Branch("simHit_Q_Truth", &m_simHit_Q_Truth);
        t_SimHit->Branch("simHit_Q_Digi", &m_simHit_Q_Digi);
        t_SimHit->Branch("simHit_module", &m_simHit_module);
        t_SimHit->Branch("simHit_stave", &m_simHit_stave);
        t_SimHit->Branch("simHit_layer", &m_simHit_layer);
        if (dettype == 1)
        {
            t_SimHit->Branch("simHit_phi", &m_simHit_phi_x);
            t_SimHit->Branch("simHit_iz", &m_simHit_z_y);
        }
        else if (dettype == 2)
        {
            t_SimHit->Branch("simHit_ix", &m_simHit_phi_x);
            t_SimHit->Branch("simHit_iy", &m_simHit_z_y);
        }
        t_SimHit->Branch("simHit_cellID", &m_simHit_cellID);
    }

    cout << "EcalDigiAlgShort::m_scale=" << m_scale << endl;
    m_geosvc = service<IGeomSvc>("GeomSvc");

    if (!m_geosvc)
        throw "EcalDigiAlgShort: Failed to find GeomSvc ...";

    m_dd4hep = m_geosvc->lcdd();

    if (!m_dd4hep)
        throw "EcalDigiAlgShort: Failed to get dd4hep::Detector ...";

    m_cellIDConverter = new rec::CellIDPositionConverter(*m_dd4hep);
    m_decoder = m_geosvc->getDecoder(_readoutName);

    if (!m_decoder)
    {
        error() << "Failed to get the decoder. " << endmsg;
        return StatusCode::FAILURE;
    }

    rndm.SetSeed(_seed);
    cout << "EcalDigiAlgShort::initialize" << endl;
    return Algorithm::initialize();
}

StatusCode EcalDigiAlgShort::execute()
{
    if (_nEvt == 0)
        cout << "EcalDigiAlgShort::execute begins..." << endl;

    if (dettype == 1)
        cout << "Barrel, processing event " << _nEvt << endl;
    else if (dettype == 2)
        cout << "End-cap, processing event " << _nEvt << endl;

    if (_nEvt < _Nskip)
    {
        ++_nEvt;
        return StatusCode::SUCCESS;
    }

    Clear();

    const edm4hep::SimCalorimeterHitCollection* SimHitCol = r_SimCaloCol.get();
    edm4hep::CalorimeterHitCollection* caloVec = w_DigiCaloCol.createAndPut();
    edm4hep::MCRecoCaloAssociationCollection* caloAssoVec = w_CaloAssociationCol.createAndPut();
    edm4hep::MCRecoCaloParticleAssociationCollection* caloMCPAssoVec = w_MCPCaloAssociationCol.createAndPut();
    vector<edm4hep::SimCalorimeterHit> m_simhitCol;
    m_simhitCol.clear();
    vector<CaloCrystalShort> m_hitCol;
    m_hitCol.clear();

    if (SimHitCol == 0)
    {
        cout << "SimCalorimeterHitCollection not found" << endl;
        return StatusCode::SUCCESS;
    }

    if (_Debug >= 1)
        cout << "Digitisation, input sim hit size = " << SimHitCol->size() << endl;

    totE_Truth = 0;
    totE_Digi = 0;

    // Merge input simhit (steps) to real simhit (crystal).
    MergeHits(*SimHitCol, m_simhitCol);
    if (_Debug >= 1)
        cout << "Hit merging finished, Nhit = " << m_simhitCol.size() << endl;

    // Loop in SimHit, digitise SimHit to DigiHit
    for (int i = 0; i < m_simhitCol.size(); ++i)
    {
        auto SimHit = m_simhitCol.at(i);

        const unsigned long long id = SimHit.getCellID();
        CaloCrystalShort hitcrystal;
        hitcrystal.setcellID(id);
        if (dettype == 1)
            hitcrystal.setcellID(m_decoder->get(id, "system"),
                                 m_decoder->get(id, "module"),
                                 m_decoder->get(id, "stave"),
                                 m_decoder->get(id, "layer"),
                                 m_decoder->get(id, "phi"),
                                 m_decoder->get(id, "z"));
        else if (dettype == 2)
            hitcrystal.setcellID(m_decoder->get(id, "system"),
                                 m_decoder->get(id, "module"),
                                 m_decoder->get(id, "stave"),
                                 m_decoder->get(id, "layer"),
                                 m_decoder->get(id, "x"),
                                 m_decoder->get(id, "y"));

        Position hitpos = m_cellIDConverter->position(id);
        TVector3 pos(10 * hitpos.x(), 10 * hitpos.y(), 10 * hitpos.z());    // cm to mm.
        hitcrystal.setPosition(pos);
        const double distcrystal = pos.Mag();

        MCParticleToEnergyWeightMap MCPEnMap;
        MCPEnMap.clear();
        vector<HitStep> Digivec;
        Digivec.clear();
        double totQ_Truth = 0;
        double totQ_Digi = 0;
        double totQ = 0;

        // Loop in all SimHitContribution (G4Step).
        for (int iCont = 0; iCont < SimHit.contributions_size(); ++iCont)
        {
            auto conb = SimHit.getContributions(iCont);
            if (!conb.isAvailable())
            {
                cout << "EcalDigiAlgShort cannot get SimHitContribution: " << iCont << endl;
                continue;
            }

            const double en = conb.getEnergy();
            if (en == 0)
                continue;

            auto mcp = conb.getParticle();
            MCPEnMap[mcp] += en;
            TVector3 steppos(conb.getStepPosition().x, conb.getStepPosition().y, conb.getStepPosition().z);
            TVector3 rpos = steppos - hitcrystal.getPosition();
            const double disthit = steppos.Mag();
            const float step_time = conb.getTime();

            m_step_x.emplace_back(steppos.x());
            m_step_y.emplace_back(steppos.y());
            m_step_z.emplace_back(steppos.z());
            m_step_t.emplace_back(step_time);
            m_step_E.emplace_back(en);
            m_stepHit_x.emplace_back(hitcrystal.getPosition().x());
            m_stepHit_y.emplace_back(hitcrystal.getPosition().y());
            m_stepHit_z.emplace_back(hitcrystal.getPosition().z());

            if (_Debug >= 3)
            {
                cout << "Cell: " << hitcrystal.getModule() << "  " << hitcrystal.getStave() << "  " << hitcrystal.getLayer() << "  " << hitcrystal.getPhiX() << "  " << hitcrystal.getZY() << endl;
                cout << "Cell position: " << hitcrystal.getPosition().x() << '\t' << hitcrystal.getPosition().y() << '\t' << hitcrystal.getPosition().z() << endl;
                cout << "Step position: " << steppos.x() << '\t' << steppos.y() << '\t' << steppos.z() << endl;
                cout << "Relative position: " << rpos.x() << '\t' << rpos.y() << '\t' << rpos.z() << endl;
            }

            // ####### For Charge Digitisation #######
            const int sign = (distcrystal >= disthit) ? 1 : -1;
            const double Ratio = exp(-(0.5 * fEcalCryLen + sign * rpos.Mag()) / Latt);
            const double Qi = en * Ratio;

            if (_Debug >= 3)
            {
                cout << Qi << endl;
                cout << sign * rpos.Mag() << endl;
            }

            totQ_Truth += Qi;

            // ####### For Time Digitisation #######
            double Ti = -1;
            int looptime = 0;
            while (Ti < 0)
            {
                Ti = Tinit + rndm.Gaus(nMat * (0.5 * fEcalCryLen + sign * rpos.Mag()) / C, Tres) + step_time;
                ++looptime;
                if (looptime > 500)
                {
                    cout << "ERROR: Time for step " << iCont << " is not positive!" << endl;
                    break;
                }
            }
            if (looptime > 500)
                continue;

            m_step_T.emplace_back(Ti);
            totQ += Qi;

            HitStep stepout;
            stepout.setQ(Qi);
            stepout.setT(Ti);
            Digivec.emplace_back(stepout);
        }

        // #######################################
        // ####### Ideal Time Digitisation #######
        // #######################################

        sort(Digivec.begin(), Digivec.end());
        double thQ = 0;
        double thT;
        for (int iCont = 0; iCont < Digivec.size(); ++iCont)
        {
            thQ += Digivec[iCont].getQ();
            if (thQ > totQ * _Qthfrac)
            {
                thT = Digivec[iCont].getT();
                if (_Debug >= 3)
                    cout << "T at index " << iCont << ": " << thT << endl;
                break;
            }
        }

        if (_UseRelDigi)
        {
            // #############################################
            // ####### Realistic Charge Digitisation #######
            // #############################################

//            const double sEcalCryMipLY = rndm.Gaus(fEcalCryMipLY, 0.1 * fEcalCryMipLY);
            const double sEcalCryMipLY = fEcalCryMipLY;

            // TODO: fEcalMIPEnergy should depend on crystal size.
            const int ScinGen = round(rndm.Poisson(totQ_Truth * 1000 / fEcalMIPEnergy * sEcalCryMipLY));

            totQ_Digi = 0.001 * EnergyDigi(ScinGen, sEcalCryMipLY);
        }
        else
        {
            if (totQ_Truth != 0)
                totQ_Digi = totQ_Truth;
            if (totQ_Digi / fEcalMIPEnergy < fEcalMIP_Thre)
                totQ_Digi = 0;
        }

        if (totQ_Digi == 0)
            continue;

        hitcrystal.setQ(totQ_Digi);
        hitcrystal.setT(thT);

        // ##################################
        // ####### Some associations  #######
        // ##################################

        // 2 hits with double-readout time.
        edm4hep::Vector3f m_pos(hitcrystal.getPosition().X(), hitcrystal.getPosition().Y(), hitcrystal.getPosition().Z());
        auto digiHit = caloVec->create();
        digiHit.setCellID(hitcrystal.getcellID());
        digiHit.setEnergy(hitcrystal.getQ());
        digiHit.setTime(hitcrystal.getT());
        digiHit.setPosition(m_pos);

        // SimHit - CaloHit association
        auto rel = caloAssoVec->create();
        rel.setRec(digiHit);
        rel.setSim(SimHit);
        rel.setWeight(1.0);

        // MCParticle - CaloHit association
//        float maxMCE = -99.;
//        edm4hep::MCParticle selMCP;
        for (auto iter: MCPEnMap)
        {
//            if (iter.second > maxMCE)
//            {
//                maxMCE = iter.second;
//                selMCP = iter.first;
//            }
            auto rel_MCP = caloMCPAssoVec->create();
            rel_MCP.setRec(digiHit);
            rel_MCP.setSim(iter.first);
            rel_MCP.setWeight(iter.second / SimHit.getEnergy());
        }

//        if (selMCP.isAvailable())
//        {
//            auto rel_MCP = caloMCPAssoVec->create();
//            rel_MCP.setRec(digiHit);
//            rel_MCP.setSim(selMCP);
//            rel_MCP.setWeight(1.0);
//        }

        // ##################################
        // ####### Writing into trees #######
        // ##################################

        m_hitCol.emplace_back(hitcrystal);
        if (hitcrystal.getQ() > 0)
            totE_Digi += hitcrystal.getQ();
        if (totQ_Truth > 0.001 * fEcalMIPEnergy * fEcalMIP_Thre)
            totE_Truth += totQ_Truth;

        if (_writeNtuple)
        {
            m_simHit_x.emplace_back(hitcrystal.getPosition().x());
            m_simHit_y.emplace_back(hitcrystal.getPosition().y());
            m_simHit_z.emplace_back(hitcrystal.getPosition().z());
            m_simHit_Q_Truth.emplace_back(totQ_Truth);
            m_simHit_Q_Digi.emplace_back(hitcrystal.getQ());
            m_simHit_T.emplace_back(hitcrystal.getT());
            m_simHit_module.emplace_back(hitcrystal.getModule());
            m_simHit_stave.emplace_back(hitcrystal.getStave());
            m_simHit_layer.emplace_back(hitcrystal.getLayer());
            m_simHit_phi_x.emplace_back(hitcrystal.getPhiX());
            m_simHit_z_y.emplace_back(hitcrystal.getZY());
            m_simHit_cellID.emplace_back(hitcrystal.getcellID());
        }
    }

    if (_writeNtuple)
    {
        t_SimCont->Fill();
        t_SimHit->Fill();
    }

    if (_Debug >= 1)
        cout << "End Loop: Hit Digitisation" << endl;

    cout << "Total Truth Energy: " << totE_Truth << endl;
    cout << "Total Digitised Energy: " << totE_Digi << endl;

    ++_nEvt;
    m_simhitCol.clear();
    return StatusCode::SUCCESS;
}

StatusCode EcalDigiAlgShort::finalize()
{
    if (_writeNtuple)
    {
        m_wfile->cd();
        t_SimCont->Write();
        t_SimHit->Write();
        m_wfile->Close();
        delete m_wfile;
    }

    info() << "Processed " << _nEvt << " events " << endmsg;
    delete m_cellIDConverter;
    return Algorithm::finalize();
}

StatusCode EcalDigiAlgShort::MergeHits(const edm4hep::SimCalorimeterHitCollection& m_col, vector<edm4hep::SimCalorimeterHit>& m_hits)
{
    m_hits.clear();
    vector<edm4hep::MutableSimCalorimeterHit> m_mergedhit;
    m_mergedhit.clear();

    for (int iter = 0; iter < m_col.size(); ++iter)
    {
        edm4hep::SimCalorimeterHit m_step = m_col[iter];
        if (!m_step.isAvailable())
        {
            cout << "ERROR HIT!" << endl;
            continue;
        }
        if (m_step.getEnergy() == 0 || m_step.contributions_size() < 1)
            continue;
        unsigned long long cellid = m_step.getCellID();
        Position hitpos = m_cellIDConverter->position(cellid);
        edm4hep::Vector3f pos(10 * hitpos.x(), 10 * hitpos.y(), 10 * hitpos.z());

        edm4hep::MutableCaloHitContribution conb;
        conb.setEnergy(m_step.getEnergy());
        conb.setStepPosition(m_step.getPosition());
        conb.setParticle(m_step.getContributions(0).getParticle());
        conb.setTime(m_step.getContributions(0).getTime());

        edm4hep::MutableSimCalorimeterHit m_hit = find(m_mergedhit, cellid);
        if (m_hit.getCellID() == 0)
        {
            m_hit.setCellID(cellid);
            m_hit.setPosition(pos);
            m_mergedhit.emplace_back(m_hit);
        }
        m_hit.addToContributions(conb);
        m_hit.setEnergy(m_hit.getEnergy() + m_step.getEnergy());
    }

    for (auto iter = m_mergedhit.begin(); iter != m_mergedhit.end(); ++iter)
    {
        edm4hep::SimCalorimeterHit constsimhit = *iter;
        m_hits.emplace_back(constsimhit);
    }

    return StatusCode::SUCCESS;
}

const double EcalDigiAlgShort::EnergyDigi(const double& ScinGen, const double& sEcalCryMipLY)
{
    const Int_t sPix = int(ScinGen);

//    if (sPix / sEcalCryMipLY < fEcalMIP_Thre)
//        return 0;
//    return sPix / sEcalCryMipLY * fEcalMIPEnergy;

    // ####### SiPM Saturation  #######
//    sPix = round(fEcalSiPMPixels * (1 - TMath::Exp(-sPix / fEcalSiPMPixels)));

    // ################################
    // ####### ADC Digitisation #######
    // ################################

    Double_t sADCMean = sPix * fEcalChargeADCMean;
    Double_t sADCSigma = TMath::Sqrt(sPix * fEcalChargeADCSigma * fEcalChargeADCSigma + fEcalNoiseADCSigma * fEcalNoiseADCSigma);
    Int_t sADC = round(rndm.Gaus(sADCMean, sADCSigma));
    Double_t sMIP;

    if (sADC <= fADCSwitch)
    {
        sADC = round(rndm.Gaus(sADC, fEcalADCError * sADC));
        sMIP = sADC / fEcalChargeADCMean / sEcalCryMipLY;
    }
    else if (sADC > fADCSwitch && int(sADC / fGainRatio_12) <= fADCSwitch)
    {
        sADCMean = sPix * fEcalChargeADCMean / fGainRatio_12;
        sADCSigma = TMath::Sqrt(sPix * fEcalChargeADCSigma / fGainRatio_12 * fEcalChargeADCSigma / fGainRatio_12 + fEcalNoiseADCSigma * fEcalNoiseADCSigma);
        sADC = round(rndm.Gaus(sADCMean, sADCSigma));
        sADC = round(rndm.Gaus(sADC, fEcalADCError * sADC));
        sMIP = sADC / fEcalChargeADCMean * fGainRatio_12 / sEcalCryMipLY;
    }
    else
    {
        sADCMean = sPix * fEcalChargeADCMean / fGainRatio_12 / fGainRatio_23;
        sADCSigma = TMath::Sqrt(sPix * fEcalChargeADCSigma / fGainRatio_12 / fGainRatio_23 * fEcalChargeADCSigma / fGainRatio_12 / fGainRatio_23 + fEcalNoiseADCSigma * fEcalNoiseADCSigma);
        sADC = round(rndm.Gaus(sADCMean, sADCSigma));
        sADC = round(rndm.Gaus(sADC, fEcalADCError * sADC));
        if (sADC > fADC - 1)
            sADC = fADC - 1;
        sMIP = sADC / fEcalChargeADCMean * fGainRatio_12 * fGainRatio_23 / sEcalCryMipLY;
    }

    return (sMIP < fEcalMIP_Thre) ? 0 : sMIP * fEcalMIPEnergy;
}

edm4hep::MutableSimCalorimeterHit EcalDigiAlgShort::find(const vector<edm4hep::MutableSimCalorimeterHit>& m_col, unsigned long long& cellid) const
{
    for (int i = 0; i < m_col.size(); ++i)
    {
        edm4hep::MutableSimCalorimeterHit hit = m_col.at(i);
        if (hit.getCellID() == cellid)
            return hit;
    }
    edm4hep::MutableSimCalorimeterHit hit;
    hit.setCellID(0);
    return hit;
}

void EcalDigiAlgShort::Clear()
{
    totE_Truth = -99;
    totE_Digi = -99;
    m_step_x.clear();
    m_step_y.clear();
    m_step_z.clear();
    m_step_t.clear();
    m_step_E.clear();
    m_stepHit_x.clear();
    m_stepHit_y.clear();
    m_stepHit_z.clear();
    m_step_T.clear();
    m_simHit_x.clear();
    m_simHit_y.clear();
    m_simHit_z.clear();
    m_simHit_T.clear();
    m_simHit_Q_Truth.clear();
    m_simHit_Q_Digi.clear();
    m_simHit_module.clear();
    m_simHit_stave.clear();
    m_simHit_layer.clear();
    m_simHit_phi_x.clear();
    m_simHit_z_y.clear();
    m_simHit_cellID.clear();
}
