import ROOT as r
import pyhepmc.io
import numpy as np
import random

#Model Dark Photon

#filename = "events_14TeV_m0.2239GeV_c0.001_to_mu_mu_s1.hepmc"
#filename = "events_14TeV_m0.01GeV_c1e-06_to_e_e_s1.hepmc"

path_txt_HepMC='/data/atlassmallfiles/users/salin/Acts_x/GEN/FASER2_GenSim/R1-L10-R1x3_DarkHiggs/txt/DarkHiggs_HepMC_file.txt'

MASSES=['0.01','0.0501','0.1585','0.3548','0.6457','0.7586','0.8913','1.2589']
MASSES=['0.1','0.2239','0.5012','0.6918','0.8128','0.955','1.5849','3.5']
DECAYS=['mu_mu']
for mass in MASSES:
    for decay in DECAYS:
# mass = '0.3548'
# decay='mu_mu'
#output_file = "Particles_testDPhoton1.root"
        output_file = f'/data/atlassmallfiles/users/salin/Acts_x/GEN/HepMC_Root/Root_DarkHiggs_X_new/Particles_DarkHiggs_m{mass}_{decay}.root'

        # Define the particle data types
        particles_dtype = {
            "event_id": np.int32,
            "particle_id": "unsigned long",
            "particle_type": "int",
            "process": "unsigned int",
            "vx": "double",
            "vy": "double",
            "vz": "double",
            "vt": "double",
            "px": "double",
            "py": "double",
            "pz": "double",
            "m": "double",
            "q": "double",
            "eta": "double",
            "phi": "double",
            "theta": "double",
            "pt": "double",
            "p": "double",
            "vertex_primary": "unsigned int",
            "vertex_secondary": "unsigned int",
            "particle": "unsigned int",
            "generation": "unsigned int",
            "sub_particle": "unsigned int"
        }

        # Create a ROOT file and tree
        output = r.TFile(output_file, "RECREATE")
        tree = r.TTree("particles", "Particle information")

        # Create vectors for each branch
        event_id = np.zeros(1, dtype=np.int32)
        particle_id = r.vector('unsigned long')()
        particle_type = r.vector('int')()
        process = r.vector('unsigned int')()
        vx = r.vector('double')()
        vy = r.vector('double')()
        vz = r.vector('double')()
        vt = r.vector('double')()
        px = r.vector('double')()
        py = r.vector('double')()
        pz = r.vector('double')()
        m = r.vector('double')()
        q = r.vector('double')()
        eta = r.vector('double')()
        phi = r.vector('double')()
        theta = r.vector('double')()
        pt = r.vector('double')()
        p = r.vector('double')()
        vertex_primary = r.vector('unsigned int')()
        vertex_secondary = r.vector('unsigned int')()
        particle = r.vector('unsigned int')()
        generation = r.vector('unsigned int')()
        sub_particle = r.vector('unsigned int')()

        # Add the branches to the tree
        tree.Branch("event_id", event_id,"event_id/i")
        tree.Branch("particle_id", particle_id)
        tree.Branch("particle_type", particle_type)
        tree.Branch("process", process)
        tree.Branch("vx", vx)
        tree.Branch("vy", vy)
        tree.Branch("vz", vz)
        tree.Branch("vt", vt)
        tree.Branch("px", px)
        tree.Branch("py", py)
        tree.Branch("pz", pz)
        tree.Branch("m", m)
        tree.Branch("q", q)
        tree.Branch("eta", eta)
        tree.Branch("phi", phi)
        tree.Branch("theta", theta)
        tree.Branch("pt", pt)
        tree.Branch("p", p)
        tree.Branch("vertex_primary", vertex_primary)
        tree.Branch("vertex_secondary", vertex_secondary)
        tree.Branch("particle", particle)
        tree.Branch("generation", generation)
        tree.Branch("sub_particle", sub_particle)        

        search_str_mass = f'events_14TeV_m{mass}GeV'
        search_str_decay = f'to_{decay}_s1.hepmc'
        #Open txt file 
        with open(path_txt_HepMC, 'r') as file:
            for line in file:
                if search_str_mass in line:   #Only text the file of a certain mass
                    if search_str_decay in line: #only text the file of events with a specific decay
                        line_without= line.replace(',', '')
                        line_without2= line_without.replace('"', '')
                        filename=line_without2.strip()
                        # Open the HepMC input file
                        with open(filename, "rb") as f:
                            ascii_reader = pyhepmc.io.ReaderAsciiHepMC2(filename)

                            # Loop over the events in the file
                            for i, event in enumerate(ascii_reader):
                                # Loop over the particles in each event
                                for j, particle_ in enumerate(event.particles):
                                    if particle_.pid in [11, -11, 13, -13, 211, -211]:
                                        # Calculate the particle's eta, phi, pt,p
                                        #IMPORTANT1 : There was a rotation in the Axis from XYZ into Z'Y'X'(new beamline along X' and Magnetic field along Z') to make ACTS work Z-> X' and X->-Z'
                                        #IMPORTANT2 Different definition ACTS and FORESEE Y->X->Z'
                                        px_, py_, pz_ = particle_.momentum.pz, -particle_.momentum.px, particle_.momentum.py    #pz-> px' px->-pz'  
                                        pt_ = (px_**2 + py_**2)**0.5
                                        p_ = (px_**2 + py_**2 + pz_**2)**0.5
                                        eta_ = -r.TMath.Log(r.TMath.Tan(0.5*r.TMath.ACos(pz_/p_)))
                                        phi_ = r.TMath.ATan2(py_, px_)
                                        theta_=r.TMath.ACos(pz_/p_)

                                        event_id[0]=i
                                        particle_id.push_back(4503599644147712 if particle_.pid > 0 else 4503599660924928)
                                        particle_type.push_back(particle_.pid)
                                        process.push_back(0)
                                        vx.push_back(particle_.production_vertex.position.z)         #vertex_x' -> vertex_z *IMPORTANT1
                                        vy.push_back(-particle_.production_vertex.position.x)         #vetex_y'>vertex_x'->-vertex_z  IMPORTANT2 and IMPORTANT1
                                        vz.push_back(particle_.production_vertex.position.y)        #vertex_z' ->-vertex_x-> vertex_y  *IMPORTANT1 and IMPORTANT2  
                                        vt.push_back(particle_.production_vertex.position.t)
                                        px.push_back(particle_.momentum.pz)                         #pz'->px
                                        py.push_back(-particle_.momentum.px)                         #py'->px'->-pz
                                        pz.push_back(particle_.momentum.py)                        #pz'->-px->py
                                        m.push_back(particle_.generated_mass)
                                        q.push_back(-1 if particle_.pid < 0 else 1)
                                        eta.push_back(eta_)
                                        phi.push_back(phi_)
                                        theta.push_back(theta_)
                                        pt.push_back(pt_)
                                        p.push_back(p_)
                                        vertex_primary.push_back(1)
                                        vertex_secondary.push_back(0)
                                        particle.push_back(2 if particle_.pid < 0 else 1)
                                        generation.push_back(0)
                                        sub_particle.push_back(0)


                                tree.Fill()
                                particle_id.clear()
                                particle_type.clear()
                                process.clear()
                                vx.clear()
                                vy.clear()
                                vz.clear()
                                vt.clear()
                                px.clear()
                                py.clear()
                                pz.clear()
                                m.clear()
                                q.clear()
                                eta.clear()
                                phi.clear()
                                theta.clear()
                                pt.clear()
                                p.clear()
                                vertex_primary.clear()
                                vertex_secondary.clear()
                                particle.clear()
                                generation.clear()
                                sub_particle.clear()


        # Write the ROOT tree to the output file and close it
        output.Write()
        output.Close()
