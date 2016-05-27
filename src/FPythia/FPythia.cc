// -*- C++ -*-
#include "AGILe/FPythia/FPythia.hh"
#include "AGILe/FPythia/FPythiaWrapper62.hh"
#include "AGILe/Utils.hh"
#include "AGILe/HepMCTools.hh"

#include "HepMC/HEPEVT_Wrapper.h"

namespace AGILe {

  // Author: Andy Buckley


  /// Standard constructor
  FPythia::FPythia() {
    _myName = "Pythia";

    /// Pythia uses HEPEVT with 4000/10000 entries and 8-byte floats
    FC_PYJETS.n = 0; //< Ensure dummyness of the next call
    call_pyhepc(1);
    HepMC::HEPEVT_Wrapper::set_max_number_entries(FC_PYDAT1.mstu[8-1]);
    //HepMC::HEPEVT_Wrapper::set_max_number_entries(10000);
    HepMC::HEPEVT_Wrapper::set_sizeof_real(8);

    // Start counting events at 1.
    _nevt = 1;

    // Initialize Pythia PYDATA block data as external
    FC_INITPYDATA();

    // Write the full shower structure
    setParam("MSTP(125)", 2);

    // Tell Pythia not to write multiple copies of particles in event record.
    setParam("MSTP(128)", 2);

    // Set up particle names map
    /// @todo Also deal with gamma/e- etc. for resolved photoproduction...
    _particleNames[ELECTRON]    = "e-";
    _particleNames[POSITRON]    = "e+";
    _particleNames[PROTON]      = "p+";
    _particleNames[ANTIPROTON]  = "pbar-";
    _particleNames[NEUTRON]     = "n0";
    _particleNames[ANTINEUTRON] = "nbar0";
    _particleNames[PHOTON]      = "gamma";
    _particleNames[MUON]        = "mu-";
    _particleNames[ANTIMUON]    = "mu+";
    // TAU, ANTITAU
    _particleNames[NU_E]        = "nu_e";
    _particleNames[NU_EBAR]     = "nu_ebar";
    _particleNames[NU_MU]       = "nu_mu";
    _particleNames[NU_MUBAR]    = "nu_mubar";
    _particleNames[NU_TAU]      = "nu_tau";
    _particleNames[NU_TAUBAR]   = "nu_taubar";
    _particleNames[PIPLUS]      = "pi+";
    _particleNames[PIMINUS]     = "pi-";
    // PIZERO
    // PHOTOELECTRON, PHOTOPOSITRON,
    // PHOTOMUON,     PHOTOANTIMUON,
    // PHOTOTAU,      PHOTOANTITAU,
    /// For Pythia, other "normal" particles are:
    /// * gamma/e-, gamma/e+, gamma/mu-, gamma/mu+, gamma/tau-, gamma/tau+, pi0
    /// Also, with crappy PDF parameterisations and other defects:
    /// * K+, K-, KS0, KL0, Lambda0, Sigma-, Sigma0, Sigma+, Xi-, Xi0, Omega
    /// And, not useable at the moment:
    /// * pomeron, reggeon
  }


  void FPythia::setGenSpecificInitialState(PdgCode p1, double e1, PdgCode p2, double e2) {
    MSG_DEBUG("Setting initial state...");
    // For Pythia, we have to hold on to this state until initialize() is called.
    _particleName1 = _particleNames[p1];
    _particleName2 = _particleNames[p2];
    /// @todo Get momenta and energies unconfused.
    _e1 = e1;
    _e2 = e2;
  }


  /// Set up default params etc.
  void FPythia::initialize() {
    Generator::initialize();
    // Make sure that PYINIT does not call PYTUNE (that should happen when the param is set).
    call_pygive("MSTP(5)=0");
    // Call pythia initialization with stored parameters as set by setGenSpecificInitialState()
    if (FC_PYPARS.mstp[142] != 1) {
      if (fuzzyEquals(_e1, _e2)) {
        call_pyinit("CMS", _particleName1.c_str(), _particleName2.c_str(), _e1+_e2);
      } else {
        stringstream p13cmd, p23cmd;
        p13cmd << "P(1,3) = " << _e1;
        p23cmd << "P(2,3) = " << -_e2;
        call_pygive("P(1,1) = 0.0");
        call_pygive("P(1,2) = 0.0");
        call_pygive(p13cmd.str().c_str());
        call_pygive("P(2,1) = 0.0");
        call_pygive("P(2,2) = 0.0");
        call_pygive(p23cmd.str().c_str());
        call_pyinit("3MOM", _particleName1.c_str(), _particleName2.c_str(), 0.0);
      }
    } else {
      // User process, parameters taken from LHAPDF initialisation file.
      call_pyinit("USER"," "," ", 0.0);
    }
    _initialized = true;
  }


  // Set a parameter with a string value
  bool FPythia::setParam(const string& name, const string& value) {
    Generator::setParam(name, value);
    const string NAME = toUpper(name);
    if (NAME == "PYTUNE" || NAME == "MSTP(5)") {
      const int tunenumber = asInt(value);
      MSG_INFO("MSTP(5) param or PYTUNE meta-param has been set: calling PYTUNE *now*");
      call_pytune(tunenumber);
    } else if (NAME == "LHEF") {
      MSG_INFO("LHEF meta-param has been set: calling OPEN *now* for LHE file = " << value);
      call_lhefopen(value.c_str());
      setParam("MSTP(161)", 88);
      setParam("MSTP(162)", 88);
    } else {
      const string pygive_input(name + "=" + value);
      call_pygive(pygive_input.c_str());
    }
    return SUCCESS;
  }


  // Run the generator for one event
  void FPythia::makeEvent(HepMC::GenEvent& evt) {
    Generator::makeEvent(evt);
    /// @todo Allow control of choice of underlying event and shower evolutions via "meta-params".
    // Generate one event.
    call_pyevnt();
    // Generate one event with new UE and shower.
    //call_pyevnw();
    // Convert common PYJETS into common HEPEVT.
    call_pyhepc(1);
    // Increment an event counter (Pythia does not count for itself).
    _nevt++;
    // fill the event using HepMC
    fillEvent(evt);
    evt.set_event_number(_nevt);
  }


  // Fill a HepMC event
  void FPythia::fillEvent(HepMC::GenEvent& evt) {
    _hepevt.fill_next_event(&evt);
    fixHepMCUnitsFromGeVmm(evt);

    // Set beam particle status = 4
    if (evt.valid_beam_particles()) {
      evt.beam_particles().first->set_status(4);
      evt.beam_particles().second->set_status(4);
    }

    // Weight(s)
    evt.weights().clear();
    if (FC_PYPARS.mstp[141] != 0) {
      evt.weights().push_back(FC_PYPARS.pari[9]);
    } else {
      evt.weights().push_back(1.0);
    }

    // Cross-section
    #ifdef HEPMC_HAS_CROSS_SECTION
    HepMC::GenCrossSection xsec;
    const double xsecval = getCrossSection();
    const double xsecerr = getCrossSection() / std::sqrt(FC_PYPARS.msti[4]);
    MSG_DEBUG("Writing cross-section = " << xsecval << " +- " << xsecerr);
    xsec.set_cross_section(xsecval, xsecerr);
    evt.set_cross_section(xsec);
    #endif

    // PDF weights, using HepMC::PdfInfo object
    /// @todo If reading from LHEF, we need to get the PDF info from LHEF instead?
    const int id1 = FC_PYPARS.msti[15-1];
    const int id2 = FC_PYPARS.msti[16-1];
    const double x1 = FC_PYPARS.pari[33-1];
    const double x2 = FC_PYPARS.pari[34-1];
    const double q = FC_PYPARS.pari[23-1];
    const double pdf1 = FC_PYPARS.pari[29-1]; // x*pdf1
    const double pdf2 = FC_PYPARS.pari[30-1]; // x*pdf2
    // std::cout << q << ": " << id1 << ", " << x1 << ", " << pdf1 << "; "
    //           << id2 << ", " << x2 << ", " << pdf2 << std::endl;
    HepMC::PdfInfo pdfi(id1, id2, x1, x2, q, pdf1, pdf2);
    evt.set_pdf_info(pdfi);
  }


  string FPythia::getPDFSet(PdgCode pid) {
    switch(pid) {
      case PROTON:
      case ANTIPROTON:
        if (FC_PYPARS.mstp[51] == 1) {
          return "PYINTERNAL";
        } else if(FC_PYPARS.mstp[51] == 2) {
          return "PYLHAPDF";
        }
        break;

      case ELECTRON:
      case POSITRON:
        if (FC_PYPARS.mstp[55] == 1) {
          return "PYINTERNAL";
        } else if(FC_PYPARS.mstp[55] == 2) {
          return "PYLHAPDF";
        }
        break;
      default:
        break;
    }
    throw runtime_error("Unknown particle for PDF set");
  }


  int FPythia::getPDFMember(PdgCode pid) {
    switch(pid) {
      case PROTON:
      case ANTIPROTON:
        return FC_PYPARS.mstp[50];
        break;
      case ELECTRON:
      case POSITRON:
        return FC_PYPARS.mstp[54];
        break;
      default:
        break;
    }
    throw runtime_error("Unknown particle for PDF set");
  }


  string FPythia::getPDFScheme(PdgCode pid) const {
    switch(pid){
      case PROTON:
      case ANTIPROTON:
        if (FC_PYPARS.mstp[51]==1) {
          return "PYINTERNAL";
        } else if(FC_PYPARS.mstp[51]==2) {
          return "LHAPDF";
        }
        break;
      case ELECTRON:
      case POSITRON:
        if (FC_PYPARS.mstp[55]==1) {
          return "PYINTERNAL";
        } else if(FC_PYPARS.mstp[55]==2) {
          return "LHAPDF";
        }
        break;
      default:
        break;
    }
    throw runtime_error("Unknown particle for PDF set");
  }


  const double FPythia::getCrossSection(){
    // _crossSection = FC_PYINT5.xsec[2][0] * 1e09;
    _crossSection = FC_PYPARS.pari[0] * 1e09;
    return _crossSection;
  }

  const string FPythia::getVersion(){
    stringstream s;
    s << FC_PYPARS.mstp[180] << "." << FC_PYPARS.mstp[181];
    return s.str();
  }

  // Tidy up after ourselves
  void FPythia::finalize() {
    MSG_DEBUG("Finalising...");
    // Print out stats from run
    call_pystat(1);
  }


}


extern "C" {
  AGILe::Generator* create() { return new AGILe::FPythia(); }
  void destroy(AGILe::Generator* gen) { delete gen; }
}
