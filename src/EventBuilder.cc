#include "EventBuilder.hh"

MiniballEventBuilder::MiniballEventBuilder( std::shared_ptr<MiniballSettings> myset ){
	
	// First get the settings
	set = myset;
	
	// No calibration file by default
	overwrite_cal = false;
	
	// No input file at the start by default
	flag_input_file = false;
	
	// Progress bar starts as false
	_prog_ = false;

	// Start at MBS event 0
	preveventid = 0;

	// ------------------------------- //
	// Initialise variables and flags  //
	// ------------------------------- //
	build_window = set->GetEventWindow();

	n_sfp.resize( set->GetNumberOfFebexSfps() );

	febex_time_start.resize( set->GetNumberOfFebexSfps() );
	febex_time_stop.resize( set->GetNumberOfFebexSfps() );
	febex_dead_time.resize( set->GetNumberOfFebexSfps() );
	pause_time.resize( set->GetNumberOfFebexSfps() );
	resume_time.resize( set->GetNumberOfFebexSfps() );
	n_board.resize( set->GetNumberOfFebexSfps() );
	n_pause.resize( set->GetNumberOfFebexSfps() );
	n_resume.resize( set->GetNumberOfFebexSfps() );
	flag_pause.resize( set->GetNumberOfFebexSfps() );
	flag_resume.resize( set->GetNumberOfFebexSfps() );

	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
		
		febex_time_start[i].resize( set->GetNumberOfFebexBoards() );
		febex_time_stop[i].resize( set->GetNumberOfFebexBoards() );
		febex_dead_time[i].resize( set->GetNumberOfFebexBoards() );
		pause_time[i].resize( set->GetNumberOfFebexBoards() );
		resume_time[i].resize( set->GetNumberOfFebexBoards() );
		n_board[i].resize( set->GetNumberOfFebexBoards() );
		n_pause[i].resize( set->GetNumberOfFebexBoards() );
		n_resume[i].resize( set->GetNumberOfFebexBoards() );
		flag_pause[i].resize( set->GetNumberOfFebexBoards() );
		flag_resume[i].resize( set->GetNumberOfFebexBoards() );
		
	}
		
}

void MiniballEventBuilder::StartFile(){
	
	// Call for every new file
	// Reset counters etc.
	
	time_prev		= 0;
	time_min		= 0;
	time_max		= 0;
	time_first		= 0;
	pulser_time		= 0;
	pulser_prev		= 0;
	ebis_prev		= 0;
	t1_prev			= 0;
	sc_prev			= 0;

	n_febex_data	= 0;
	n_info_data		= 0;

	n_pulser		= 0;
	n_ebis			= 0;
	n_t1			= 0;
	n_sc			= 0;

	n_miniball		= 0;
	n_cd			= 0;
	n_bd			= 0;
	n_spede			= 0;
	n_ic			= 0;

	gamma_ctr		= 0;
	gamma_ab_ctr	= 0;
	cd_ctr			= 0;
	bd_ctr			= 0;
	spede_ctr		= 0;
	ic_ctr			= 0;

	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {

		n_sfp[i] = 0;

		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {

			febex_time_start[i][j] = 0;
			febex_time_stop[i][j] = 0;
			febex_dead_time[i][j] = 0;
			pause_time[i][j] = 0;
			resume_time[i][j] = 0;
			n_board[i][j] = 0;
			n_pause[i][j] = 0;
			n_resume[i][j] = 0;
			flag_pause[i][j] = false;
			flag_resume[i][j] = false;

		}
	
	}
		
}

void MiniballEventBuilder::SetInputFile( std::string input_file_name ) {
		
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}
	
	flag_input_file = true;
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("mb_sort") );
	SetMBSInfoTree( (TTree*)input_file->Get("mbsinfo") );
	StartFile();

	return;
	
}

void MiniballEventBuilder::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = user_tree;
	in_data = nullptr;
	input_tree->SetBranchAddress( "data", &in_data );

	return;
	
}

void MiniballEventBuilder::SetMBSInfoTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	mbsinfo_tree = user_tree;
	mbs_info = nullptr;
	mbsinfo_tree->SetBranchAddress( "mbsinfo", &mbs_info );

	return;

}

void MiniballEventBuilder::SetOutput( std::string output_file_name ) {

	// These are the branches we need
	write_evts = std::make_unique<MiniballEvts>();
	gamma_evt = std::make_shared<GammaRayEvt>();
	gamma_ab_evt = std::make_shared<GammaRayAddbackEvt>();
	particle_evt = std::make_shared<ParticleEvt>();
	spede_evt = std::make_shared<SpedeEvt>();
	bd_evt = std::make_shared<BeamDumpEvt>();
	ic_evt = std::make_shared<IonChamberEvt>();

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "MiniballEvts", "MiniballEvts", write_evts.get() );
	output_tree->SetAutoFlush();

	// Create log file.
	std::string log_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	log_file_name += ".log";
	log_file.open( log_file_name.data(), std::ios::app );

	// Hisograms in separate function
	MakeEventHists();
	
}

void MiniballEventBuilder::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;
	event_open = false;

	hit_ctr = 0;
	
	mb_en_list.clear();
	mb_ts_list.clear();
	mb_clu_list.clear();
	mb_cry_list.clear();
	mb_seg_list.clear();
	
	std::vector<float>().swap(mb_en_list);
	std::vector<unsigned long long>().swap(mb_ts_list);
	std::vector<unsigned char>().swap(mb_clu_list);
	std::vector<unsigned char>().swap(mb_cry_list);
	std::vector<unsigned char>().swap(mb_seg_list);
	
	cd_en_list.clear();
	cd_ts_list.clear();
	cd_det_list.clear();
	cd_sec_list.clear();
	cd_side_list.clear();
	cd_strip_list.clear();
	
	std::vector<float>().swap(cd_en_list);
	std::vector<unsigned long long>().swap(cd_ts_list);
	std::vector<unsigned char>().swap(cd_det_list);
	std::vector<unsigned char>().swap(cd_sec_list);
	std::vector<unsigned char>().swap(cd_side_list);
	std::vector<unsigned char>().swap(cd_strip_list);
	
	bd_en_list.clear();
	bd_ts_list.clear();
	bd_det_list.clear();
	
	std::vector<float>().swap(bd_en_list);
	std::vector<unsigned long long>().swap(bd_ts_list);
	std::vector<unsigned char>().swap(bd_det_list);

	spede_en_list.clear();
	spede_ts_list.clear();
	spede_seg_list.clear();
	
	std::vector<float>().swap(spede_en_list);
	std::vector<unsigned long long>().swap(spede_ts_list);
	std::vector<unsigned char>().swap(spede_seg_list);

	ic_en_list.clear();
	ic_ts_list.clear();
	ic_id_list.clear();
	
	std::vector<float>().swap(ic_en_list);
	std::vector<unsigned long long>().swap(ic_ts_list);
	std::vector<unsigned char>().swap(ic_id_list);

	write_evts->ClearEvt();
	
	return;
	
}


void MiniballEventBuilder::MakeEventHists(){
	
	std::string hname, htitle;
	std::string dirname;
	
	// ----------------- //
	// Timing histograms //
	// ----------------- //
	dirname =  "timing";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	tdiff = new TH1F( "tdiff", "Time difference to first trigger;#Delta t [ns]", 1e3, -10, 1e5 );
	tdiff_clean = new TH1F( "tdiff_clean", "Time difference to first trigger without noise;#Delta t [ns]", 1e3, -10, 1e5 );

	pulser_freq = new TProfile( "pulser_freq", "Frequency of pulser in FEBEX DAQ as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	pulser_period = new TH1F( "pulser_period", "Period of pulser in FEBEX DAQ;T [ns]", 10e3, 0, 10e9 );
	ebis_freq = new TProfile( "ebis_freq", "Frequency of EBIS events as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	ebis_period = new TH1F( "ebis_period", "Period of EBIS events;T [ns]", 10e3, 0, 10e9 );
	t1_freq = new TProfile( "t1_freq", "Frequency of T1 events (p+ on ISOLDE target) as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	t1_period = new TH1F( "t1_period", "Period of T1 events (p+ on ISOLDE target);T [ns]", 10e3, 0, 10e9 );
	sc_freq = new TProfile( "sc_freq", "Frequency of SuperCycle events as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	sc_period = new TH1F( "sc_period", "Period of SuperCycle events;T [ns]", 10e3, 0, 10e9 );

	// ------------------- //
	// Miniball histograms //
	// ------------------- //
	dirname = "miniball";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	mb_td_core_seg  = new TH1F( "mb_td_core_seg",  "Time difference between core and segment in same crystal;#Delta t [ns]", 499, -2495, 2495 );
	mb_td_core_core = new TH1F( "mb_td_core_core", "Time difference between two cores in same cluster;#Delta t [ns]", 499, -2495, 2495 );

	mb_en_core_seg.resize( set->GetNumberOfMiniballClusters() );
	mb_en_core_seg_ebis_on.resize( set->GetNumberOfMiniballClusters() );

	for( unsigned int i = 0; i < set->GetNumberOfMiniballClusters(); ++i ) {
		
		dirname = "miniball/cluster_" + std::to_string(i);
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		mb_en_core_seg[i].resize( set->GetNumberOfMiniballCrystals() );
		mb_en_core_seg_ebis_on[i].resize( set->GetNumberOfMiniballCrystals() );

		for( unsigned int j = 0; j < set->GetNumberOfMiniballCrystals(); ++j ) {

				hname  = "mb_en_core_seg_" + std::to_string(i) + "_";
				hname += std::to_string(j);
				htitle  = "Gamma-ray spectrum from cluster " + std::to_string(i);
				htitle += " core " + std::to_string(j) + ", gated by segment ";
				htitle += ";segment ID;Energy (keV)";
				mb_en_core_seg[i][j] = new TH2F( hname.data(), htitle.data(), 7, -0.5, 6.5, 4096, -0.5, 4095.5 );
				
				hname  = "mb_en_core_seg_" + std::to_string(i) + "_";
				hname += std::to_string(j) + "_ebis_on";
				htitle  = "Gamma-ray spectrum from cluster " + std::to_string(i);
				htitle += " core " + std::to_string(j) + ", gated by segment ";
				htitle += " gated by EBIS time (1.5 ms);segment ID;Energy (keV)";
				mb_en_core_seg_ebis_on[i][j] = new TH2F( hname.data(), htitle.data(), 7, -0.5, 6.5, 4096, -0.5, 4095.5 );
			
		} // j
		
	} // i
	
	// ------------- //
	// CD histograms //
	// ------------- //
	dirname = "cd";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	cd_pen_id.resize( set->GetNumberOfCDDetectors() );
	cd_nen_id.resize( set->GetNumberOfCDDetectors() );
	cd_pn_1v1.resize( set->GetNumberOfCDDetectors() );
	cd_pn_1v2.resize( set->GetNumberOfCDDetectors() );
	cd_pn_2v1.resize( set->GetNumberOfCDDetectors() );
	cd_pn_2v2.resize( set->GetNumberOfCDDetectors() );
	cd_pn_td.resize( set->GetNumberOfCDDetectors() );
	cd_pp_td.resize( set->GetNumberOfCDDetectors() );
	cd_nn_td.resize( set->GetNumberOfCDDetectors() );
	cd_pn_mult.resize( set->GetNumberOfCDDetectors() );

	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ) {
		
		cd_pen_id[i].resize( set->GetNumberOfCDSectors() );
		cd_nen_id[i].resize( set->GetNumberOfCDSectors() );
		cd_pn_1v1[i].resize( set->GetNumberOfCDSectors() );
		cd_pn_1v2[i].resize( set->GetNumberOfCDSectors() );
		cd_pn_2v1[i].resize( set->GetNumberOfCDSectors() );
		cd_pn_2v2[i].resize( set->GetNumberOfCDSectors() );
		cd_pn_td[i].resize( set->GetNumberOfCDSectors() );
		cd_pp_td[i].resize( set->GetNumberOfCDSectors() );
		cd_nn_td[i].resize( set->GetNumberOfCDSectors() );
		cd_pn_mult[i].resize( set->GetNumberOfCDSectors() );

		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ) {
			
			hname  = "cd_pen_id_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD p-side energy for sector " + std::to_string(i);
			htitle += ";Strip ID;Energy (keV);Counts per strip, per 50 keV";
			cd_pen_id[i][j] = new TH2F( hname.data(), htitle.data(),
									   set->GetNumberOfCDPStrips(), -0.5, set->GetNumberOfCDPStrips(),
									   4000, 0, 2000e3 );
			
			hname  = "cd_nen_id_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD n-side energy for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";Strip ID;Energy (keV);Counts per strip, per 50 keV";
			cd_nen_id[i][j] = new TH2F( hname.data(), htitle.data(),
									   set->GetNumberOfCDPStrips(), -0.5, set->GetNumberOfCDPStrips(),
									   4000, 0, 2000e3 );
			
			hname  = "cd_pn_1v1_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD p-side vs n-side energy, multiplicity 1v1";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";p-side Energy (keV);n-side Energy (keV);Counts";
			cd_pn_1v1[i][j] = new TH2F( hname.data(), htitle.data(), 4000, 0, 2000e3, 400, 0, 2000e3 );
			
			hname  = "cd_pn_1v2_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD p-side vs n-side energy, multiplicity 1v2";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";p-side Energy (keV);n-side Energy (keV);Counts";
			cd_pn_1v2[i][j] = new TH2F( hname.data(), htitle.data(), 4000, 0, 2000e3, 400, 0, 2000e3 );
			
			hname  = "cd_pn_2v1_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD p-side vs n-side energy, multiplicity 2v1";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";p-side Energy (keV);n-side Energy (keV);Counts";
			cd_pn_2v1[i][j] = new TH2F( hname.data(), htitle.data(), 4000, 0, 2000e3, 400, 0, 2000e3 );
			
			hname  = "cd_pn_2v2_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD p-side vs n-side energy, multiplicity 2v2";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";p-side Energy (keV);n-side Energy (keV);Counts";
			cd_pn_2v2[i][j] = new TH2F( hname.data(), htitle.data(), 4000, 0, 2000e3, 400, 0, 2000e3 );
			
			hname  = "cd_pn_td_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD p-side vs n-side time difference ";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";time difference (ns);Counts per 10 ns";
			cd_pn_td[i][j] = new TH1F( hname.data(), htitle.data(), 799, -4e3, 4e3 );
			
			hname  = "cd_pp_td_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD p-side vs p-side time difference ";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";time difference (ns);Counts per 10 ns";
			cd_pp_td[i][j] = new TH1F( hname.data(), htitle.data(), 799, -4e3, 4e3 );
			
			hname  = "cd_nn_td_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD n-side vs n-side time difference ";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";time difference (ns);Counts per 10 ns";
			cd_nn_td[i][j] = new TH1F( hname.data(), htitle.data(), 799, -4e3, 4e3 );
			
			hname  = "cd_pn_mult_" + std::to_string(i) + "_" + std::to_string(j);
			htitle  = "CD n-side vs n-side multiplicity ";
			htitle += "for detector " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";p-side miltiplicity;n-side miltiplicity";
			cd_pn_mult[i][j] = new TH2F( hname.data(), htitle.data(), 10, -0.5, 9.5, 10, -0.5, 9.5 );
			
		} // j
		
	} // i
	
	
	// --------------------- //
	// IonChamber histograms //
	// --------------------- //
	dirname = "ic";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	ic_td = new TH1F( "ic_td", "Time difference between signals in the ion chamber;#Delta t [ns]", 499, -2495, 2495 );
	ic_dE = new TH1F( "ic_dE", "Ionisation chamber;Energy in first layer (Gas) (arb. units);Counts", 4096, 0, 10000 );
	ic_E = new TH1F( "ic_E", "Ionisation chamber;Energy in final layer (Si) (arb. units);Counts", 4096, 0, 10000 );
	ic_dE_E = new TH2F( "ic_dE_E", "Ionisation chamber;Rest energy, E (arb. units);Energy Loss, dE (arb. units);Counts", 4096, 0, 10000, 4096, 0, 10000 );

	return;
	
}


void MiniballEventBuilder::GammaRayFinder() {
	
	// Temporary variables for addback
	unsigned long long MaxTime; // time of event with maximum energy
	unsigned char MaxSegId; // segment with maximum energy
	unsigned char MaxCryId; // crystal with maximum energy
	float MaxEnergy; // maximum core energy
	float MaxSegEnergy; // maximum segment energy
	float SegSumEnergy; // add segment energies
	float AbSumEnergy; // add core energies for addback
	unsigned char seg_mul; // segment multiplicity
	unsigned char ab_mul; // addback multiplicity
	std::vector<unsigned char> ab_index; // index of addback already used
	bool skip_event; // has this event been used already
	
	// Loop over all the events in Miniball detectors
	for( unsigned int i = 0; i < mb_en_list.size(); ++i ) {
	
		// Check if it's a core event
		if( mb_seg_list.at(i) != 0 ) continue;
		
		// Reset addback variables
		MaxSegId = 0; // initialise as core (if no segment hit (dead), use core!)
		MaxSegEnergy = 0.;
		SegSumEnergy = 0.;
		seg_mul = 0;
		
		// Loop again to find the matching segments
		for( unsigned int j = 0; j < mb_en_list.size(); ++j ) {

			// Skip if it's not the same crystal and cluster
			if( mb_clu_list.at(i) != mb_clu_list.at(j) ||
			    mb_cry_list.at(i) != mb_cry_list.at(j) ) continue;
			
			// Fill the segment spectra with core energies
			mb_en_core_seg[mb_clu_list.at(i)][mb_cry_list.at(i)]->Fill( mb_seg_list.at(j), mb_en_list.at(i) );
			if( mb_ts_list.at(j) - ebis_time < 1.5e6 )
				mb_en_core_seg_ebis_on[mb_clu_list.at(i)][mb_cry_list.at(i)]->Fill( mb_seg_list.at(j), mb_en_list.at(i) );

			// Skip if it's a core again, also fill time diff plot
			if( i == j || mb_seg_list.at(j) == 0 ) {
				
				// Fill the time difference spectrum
				mb_td_core_core->Fill( (long long)mb_ts_list.at(i) - (long long)mb_ts_list.at(j) );
				continue;
			
			}
			
			// Increment the segment multiplicity and sum energy
			seg_mul++;
			SegSumEnergy += mb_en_list.at(j);
			
			// Is this bigger than the current maximum energy?
			if( mb_en_list.at(j) > MaxSegEnergy ){
				
				MaxSegEnergy = mb_en_list.at(j);
				MaxSegId = mb_seg_list.at(j);
				
			}
			
			// Fill the time difference spectrum
			mb_td_core_seg->Fill( (long long)mb_ts_list.at(i) - (long long)mb_ts_list.at(j) );
			
		} // j: matching segments
		
		// Build the single crystal gamma-ray event
		gamma_ctr++;
		gamma_evt->SetEnergy( mb_en_list.at(i) );
		gamma_evt->SetSegmentEnergy( MaxSegEnergy );
		gamma_evt->SetCluster( mb_clu_list.at(i) );
		gamma_evt->SetCrystal( mb_cry_list.at(i) );
		gamma_evt->SetSegment( MaxSegId );
		gamma_evt->SetTime( mb_ts_list.at(i) );
		write_evts->AddEvt( gamma_evt );

	} // i: core events
	
	
	// Loop over all the gamma-ray singles for addback
	for( unsigned int i = 0; i < write_evts->GetGammaRayMultiplicity(); ++i ) {

		// Reset addback variables
		AbSumEnergy = write_evts->GetGammaRayEvt(i)->GetEnergy();
		MaxCryId = write_evts->GetGammaRayEvt(i)->GetCrystal();
		MaxSegId = write_evts->GetGammaRayEvt(i)->GetSegment();
		MaxEnergy = AbSumEnergy;
		MaxSegEnergy = write_evts->GetGammaRayEvt(i)->GetSegmentEnergy();
		MaxTime = write_evts->GetGammaRayEvt(i)->GetTime();
		ab_mul = 1;	// this is already the first event
		
		// Check we haven't already used this event
		skip_event = false;
		for( unsigned int k = 0; k < ab_index.size(); ++k )
			if( ab_index.at(k) == i ) skip_event = true;
		if( skip_event ) continue;

		// Loop to find a matching event for addback
		for( unsigned int j = i+1; j < write_evts->GetGammaRayMultiplicity(); ++j ) {

			// Make sure we are in the same cluster
			// In the future we might consider a more intelligent
			// algorithm, which uses the line-of-sight idea
			if( write_evts->GetGammaRayEvt(i)->GetCluster() !=
				write_evts->GetGammaRayEvt(j)->GetCluster() ) continue;
			
			// Check we haven't already used this event
			skip_event = false;
			for( unsigned int k = 0; k < ab_index.size(); ++k )
				if( ab_index.at(k) == j ) skip_event = true;
			if( skip_event ) continue;
			
			// Then we can add them back
			ab_mul++;
			AbSumEnergy += write_evts->GetGammaRayEvt(j)->GetEnergy();
			ab_index.push_back(j);

			// Is this bigger than the current maximum energy?
			if( write_evts->GetGammaRayEvt(j)->GetEnergy() > MaxEnergy ){
				
				MaxEnergy = write_evts->GetGammaRayEvt(j)->GetEnergy();
				MaxSegEnergy = write_evts->GetGammaRayEvt(j)->GetEnergy();
				MaxCryId = write_evts->GetGammaRayEvt(j)->GetCrystal();
				MaxSegId = write_evts->GetGammaRayEvt(j)->GetSegment();
				MaxTime = write_evts->GetGammaRayEvt(j)->GetTime();

			}

		} // j: loop for matching addback

		// Build the single crystal gamma-ray event
		gamma_ab_ctr++;
		gamma_ab_evt->SetEnergy( AbSumEnergy );
		gamma_ab_evt->SetSegmentEnergy( MaxSegEnergy );
		gamma_ab_evt->SetCluster( write_evts->GetGammaRayEvt(i)->GetCluster() );
		gamma_ab_evt->SetCrystal( MaxCryId );
		gamma_ab_evt->SetSegment( MaxSegId );
		gamma_ab_evt->SetTime( MaxTime );
		write_evts->AddEvt( gamma_ab_evt );
		
	} // i: gamma-ray singles
	
	return;
	
}


void MiniballEventBuilder::ParticleFinder() {

	// Variables for the finder algorithm
	std::vector<unsigned char> pindex;
	std::vector<unsigned char> nindex;

	// Loop over each detector and sector
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ){

		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ){
			
			// Reset variables for a new detector element
			pindex.clear();
			nindex.clear();
			std::vector<unsigned char>().swap(pindex);
			std::vector<unsigned char>().swap(nindex);
			int pmax_idx = -1, nmax_idx = -1;
			float pmax_en = -999., nmax_en = -999.;
			float psum_en, nsum_en;
			
			// Calculate p/n side multiplicities and get indicies
			for( unsigned int k = 0; k < cd_en_list.size(); ++k ){
				
				// Test that we have the correct detector and quadrant
				if( i != cd_det_list.at(k) || j != cd_sec_list.at(k) )
					continue;

				// Check max energy and push back the multiplicity
				if( cd_side_list.at(k) == 0 ) {
				
					pindex.push_back(k);
					
					// Check if it is max energy
					if( cd_en_list.at(k) > pmax_en ){
					
						pmax_en = cd_en_list.at(k);
						pmax_idx = k;
					
					}

				
				} // p-side
				
				else if( cd_side_list.at(k) == 1 ) {
					
					nindex.push_back(k);
				
					// Check if it is max energy
					if( cd_en_list.at(k) > nmax_en ){
					
						nmax_en = cd_en_list.at(k);
						nmax_idx = k;
					
					}

				} // n-side
			
			} // k: all CD events
			
			
			// Plot multiplcities
			if( pindex.size() || nindex.size() )
				cd_pn_mult[i][j]->Fill( pindex.size(), nindex.size() );
			
			// Plot time differences
			for( unsigned int p1 = 0; p1 < pindex.size(); ++p1 ){

				for( unsigned int n1 = 0; n1 < nindex.size(); ++n1 ){
					
					cd_pn_td[i][j]->Fill( (double)cd_ts_list.at( pindex[p1] ) -
										  (double)cd_ts_list.at( nindex[n1] ) );
					
				} // n1
				
				for( unsigned int p2 = p1+1; p2 < pindex.size(); ++p2 ){
					
					cd_pp_td[i][j]->Fill( (double)cd_ts_list.at( pindex[p1] ) -
										  (double)cd_ts_list.at( pindex[p2] ) );
					
				} // p2
				
			} // p1
			
			for( unsigned int n1 = 0; n1 < nindex.size(); ++n1 ){

				for( unsigned int n2 = n1+1; n2 < nindex.size(); ++n2 ){
					
					cd_nn_td[i][j]->Fill( (double)cd_ts_list.at( nindex[n1] ) -
										  (double)cd_ts_list.at( nindex[n2] ) );

				} // n2

			} // n1
			
			// ----------------------- //
			// Particle reconstruction //
			// ----------------------- //
			// 1 vs 1 - easiest situation
			if( pindex.size() == 1 && nindex.size() == 1 ) {

				// Set event
				particle_evt->SetEnergyP( cd_en_list.at( pindex[0] ) );
				particle_evt->SetEnergyN( cd_en_list.at( nindex[0] ) );
				particle_evt->SetTimeP( cd_ts_list.at( pindex[0] ) );
				particle_evt->SetTimeN( cd_ts_list.at( nindex[0] ) );
				particle_evt->SetDetector( i );
				particle_evt->SetSector( j );
				particle_evt->SetStripP( cd_strip_list.at( pindex[0] ) );
				particle_evt->SetStripN( cd_strip_list.at( nindex[0] ) );

				// Fill tree
				write_evts->AddEvt( particle_evt );
				cd_ctr++;

				// Fill histograms
				cd_pen_id[i][j]->Fill( cd_strip_list.at( pindex[0] ),
									  cd_en_list.at( pindex[0] ) );
				cd_nen_id[i][j]->Fill( cd_strip_list.at( nindex[0] ),
									  cd_en_list.at( nindex[0] ) );
				cd_pn_1v1[i][j]->Fill( cd_en_list.at( pindex[0] ),
									  cd_en_list.at( nindex[0] ) );

			} // 1 vs 1
			
			// 1 vs 2 - n-side charge sharing?
			if( pindex.size() == 1 && nindex.size() == 2 ) {

				// Neighbour strips
				if( TMath::Abs( cd_strip_list.at( nindex[0] ) - cd_strip_list.at( nindex[1] ) ) == 1 ) {

					// Simple sum of both energies, cross-talk not included yet
					nsum_en  = cd_en_list.at( nindex[0] );
					nsum_en += cd_en_list.at( nindex[1] );
					
					// Set event
					particle_evt->SetEnergyP( cd_en_list.at( pindex[0] ) );
					particle_evt->SetEnergyN( nsum_en );
					particle_evt->SetTimeP( cd_ts_list.at( pindex[0] ) );
					particle_evt->SetTimeN( cd_ts_list.at( nmax_idx ) );
					particle_evt->SetDetector( i );
					particle_evt->SetSector( j );
					particle_evt->SetStripP( cd_strip_list.at( pindex[0] ) );
					particle_evt->SetStripN( cd_strip_list.at( nmax_idx ) );

					// Fill tree
					write_evts->AddEvt( particle_evt );
					cd_ctr++;

					// Fill histograms
					cd_pen_id[i][j]->Fill( cd_strip_list.at( pindex[0] ),
										  cd_en_list.at( pindex[0] ) );
					cd_nen_id[i][j]->Fill( nsum_en,
										  cd_en_list.at( nmax_idx ) );
					cd_pn_1v2[i][j]->Fill( cd_en_list.at( pindex[0] ),
										  cd_en_list.at( nindex[0] ) );
					cd_pn_1v2[i][j]->Fill( cd_en_list.at( pindex[0] ),
										  cd_en_list.at( nindex[1] ) );

				} // neighbour strips
				
				// otherwise treat as 1 vs 1
				else {
					
					// Set event
					particle_evt->SetEnergyP( cd_en_list.at( pindex[0] ) );
					particle_evt->SetEnergyN( cd_en_list.at( nmax_idx ) );
					particle_evt->SetTimeP( cd_ts_list.at( pindex[0] ) );
					particle_evt->SetTimeN( cd_ts_list.at( nmax_idx ) );
					particle_evt->SetDetector( i );
					particle_evt->SetSector( j );
					particle_evt->SetStripP( cd_strip_list.at( pindex[0] ) );
					particle_evt->SetStripN( cd_strip_list.at( nmax_idx ) );

					// Fill tree
					write_evts->AddEvt( particle_evt );
					cd_ctr++;

					// Fill histograms
					cd_pen_id[i][j]->Fill( cd_strip_list.at( pindex[0] ),
										  cd_en_list.at( pindex[0] ) );
					cd_nen_id[i][j]->Fill( cd_strip_list.at( nmax_idx ),
										  cd_en_list.at( nmax_idx ) );

					
				} // treat as 1 vs 1

			} // 1 vs 2
			
			// 2 vs 1 - p-side charge sharing?
			if( pindex.size() == 2 && nindex.size() == 1 ) {

				// Neighbour strips
				if( TMath::Abs( cd_strip_list.at( pindex[0] ) - cd_strip_list.at( pindex[1] ) ) == 1 ) {

					// Simple sum of both energies, cross-talk not included yet
					psum_en  = cd_en_list.at( pindex[0] );
					psum_en += cd_en_list.at( pindex[1] );
					
					// Set event
					particle_evt->SetEnergyP( psum_en );
					particle_evt->SetEnergyN( cd_en_list.at( nindex[0] ) );
					particle_evt->SetTimeP( cd_ts_list.at( pmax_idx ) );
					particle_evt->SetTimeN( cd_ts_list.at( nindex[0] ) );
					particle_evt->SetDetector( i );
					particle_evt->SetSector( j );
					particle_evt->SetStripP( cd_strip_list.at( pmax_idx ) );
					particle_evt->SetStripN( cd_strip_list.at( nindex[0] ) );

					// Fill tree
					write_evts->AddEvt( particle_evt );
					cd_ctr++;

					// Fill histograms
					cd_pen_id[i][j]->Fill( psum_en,
										  cd_en_list.at( pmax_idx ) );
					cd_nen_id[i][j]->Fill( cd_strip_list.at( nindex[0] ),
										  cd_en_list.at( nindex[0] ) );
					cd_pn_2v1[i][j]->Fill( cd_en_list.at( pindex[0] ),
										  cd_en_list.at( nindex[0] ) );
					cd_pn_2v1[i][j]->Fill( cd_en_list.at( pindex[1] ),
										  cd_en_list.at( nindex[0] ) );

				} // neighbour strips

				// otherwise treat as 1 vs 1
				else {
					
					// Set event
					particle_evt->SetEnergyP( cd_en_list.at( pmax_idx ) );
					particle_evt->SetEnergyN( cd_en_list.at( nindex[0] ) );
					particle_evt->SetTimeP( cd_ts_list.at( pmax_idx ) );
					particle_evt->SetTimeN( cd_ts_list.at( nindex[0] ) );
					particle_evt->SetDetector( i );
					particle_evt->SetSector( j );
					particle_evt->SetStripP( cd_strip_list.at( pmax_idx ) );
					particle_evt->SetStripN( cd_strip_list.at( nindex[0] ) );

					// Fill tree
					write_evts->AddEvt( particle_evt );
					cd_ctr++;

					// Fill histograms
					cd_pen_id[i][j]->Fill( cd_strip_list.at( pmax_idx ),
										  cd_en_list.at( pmax_idx ) );
					cd_nen_id[i][j]->Fill( cd_strip_list.at( nindex[0] ),
										  cd_en_list.at( nindex[0] ) );

					
				} // treat as 1 vs 1

			} // 2 vs 1
			
			// 2 vs 2 - charge sharing on both or two particles?
			if( pindex.size() == 2 && nindex.size() == 2 ) {

				// Neighbour strips - p-side + n-side
				if( TMath::Abs( cd_strip_list.at( pindex[0] ) - cd_strip_list.at( pindex[1] ) ) == 1 &&
				    TMath::Abs( cd_strip_list.at( nindex[0] ) - cd_strip_list.at( nindex[1] ) ) == 1 ) {

					// Simple sum of both energies, cross-talk not included yet
					psum_en  = cd_en_list.at( pindex[0] );
					psum_en += cd_en_list.at( pindex[1] );
					nsum_en  = cd_en_list.at( nindex[0] );
					nsum_en += cd_en_list.at( nindex[1] );

					// Set event
					particle_evt->SetEnergyP( psum_en );
					particle_evt->SetEnergyN( nsum_en );
					particle_evt->SetTimeP( cd_ts_list.at( pmax_idx ) );
					particle_evt->SetTimeN( cd_ts_list.at( nmax_idx ) );
					particle_evt->SetDetector( i );
					particle_evt->SetSector( j );
					particle_evt->SetStripP( cd_strip_list.at( pmax_idx ) );
					particle_evt->SetStripN( cd_strip_list.at( nmax_idx ) );

					// Fill tree
					write_evts->AddEvt( particle_evt );
					cd_ctr++;

					// Fill histograms
					cd_pen_id[i][j]->Fill( cd_strip_list.at( pmax_idx ),
										  psum_en );
					cd_nen_id[i][j]->Fill( cd_strip_list.at( nmax_idx ),
										  nsum_en );
					cd_pn_2v2[i][j]->Fill( cd_en_list.at( pindex[0] ),
										  cd_en_list.at( nindex[0] ) );
					cd_pn_2v2[i][j]->Fill( cd_en_list.at( pindex[0] ),
										  cd_en_list.at( nindex[1] ) );
					cd_pn_2v2[i][j]->Fill( cd_en_list.at( pindex[1] ),
										  cd_en_list.at( nindex[0] ) );
					cd_pn_2v2[i][j]->Fill( cd_en_list.at( pindex[1] ),
										  cd_en_list.at( nindex[1] ) );

				} // neighbour strips - p-side + n-side

				// Neighbour strips - p-side only
				else if( TMath::Abs( cd_strip_list.at( pindex[0] ) - cd_strip_list.at( pindex[1] ) ) == 1 ) {

					// Simple sum of both energies, cross-talk not included yet
					psum_en  = cd_en_list.at( pindex.at(0) );
					psum_en += cd_en_list.at( pindex.at(1) );

					// Set event
					particle_evt->SetEnergyP( psum_en );
					particle_evt->SetEnergyN( nmax_en );
					particle_evt->SetTimeP( cd_ts_list.at( pmax_idx ) );
					particle_evt->SetTimeN( cd_ts_list.at( nmax_idx ) );
					particle_evt->SetDetector( i );
					particle_evt->SetSector( j );
					particle_evt->SetStripP( cd_strip_list.at( pmax_idx ) );
					particle_evt->SetStripN( cd_strip_list.at( nmax_idx ) );

					// Fill tree
					write_evts->AddEvt( particle_evt );
					cd_ctr++;

					// Fill histograms
					cd_pen_id[i][j]->Fill( cd_strip_list.at( pmax_idx ),
										  psum_en );
					cd_nen_id[i][j]->Fill( cd_strip_list.at( nmax_idx ),
										  cd_en_list.at( nmax_idx ) );
					
				} // neighbour strips - p-side only

				// Neighbour strips - n-side only
				if( TMath::Abs( cd_strip_list.at( nindex[0] ) - cd_strip_list.at( nindex[1] ) ) == 1 ) {

					// Simple sum of both energies, cross-talk not included yet
					nsum_en  = cd_en_list.at( nindex.at(0) );
					nsum_en += cd_en_list.at( nindex.at(1) );

					// Set event
					particle_evt->SetEnergyP( cd_en_list.at( pmax_idx ) );
					particle_evt->SetEnergyN( nsum_en );
					particle_evt->SetTimeP( cd_ts_list.at( pmax_idx ) );
					particle_evt->SetTimeN( cd_ts_list.at( nmax_idx ) );
					particle_evt->SetDetector( i );
					particle_evt->SetSector( j );
					particle_evt->SetStripP( cd_strip_list.at( pmax_idx ) );
					particle_evt->SetStripN( cd_strip_list.at( nmax_idx ) );

					// Fill tree
					write_evts->AddEvt( particle_evt );
					cd_ctr++;

					// Fill histograms
					cd_pen_id[i][j]->Fill( cd_strip_list.at( pmax_idx ),
										  cd_en_list.at( pmax_idx ) );
					cd_nen_id[i][j]->Fill( cd_strip_list.at( nmax_idx ),
										  nsum_en );

				} // neighbour strips - n-side only

			} // 2 vs 2
			
			// Everything else, just take the max energy for now
			else if( pmax_idx >= 0 && nmax_idx >= 0 ){
				
				// Set event
				particle_evt->SetEnergyP( pmax_en );
				particle_evt->SetEnergyN( nmax_en );
				particle_evt->SetTimeP( cd_ts_list.at( pmax_idx ) );
				particle_evt->SetTimeN( cd_ts_list.at( nmax_idx ) );
				particle_evt->SetDetector( i );
				particle_evt->SetSector( j );
				particle_evt->SetStripP( cd_strip_list.at( pmax_idx ) );
				particle_evt->SetStripN( cd_strip_list.at( nmax_idx ) );

				// Fill tree
				write_evts->AddEvt( particle_evt );
				cd_ctr++;
				
			}
			
		} // j: sector ID
		
	} // i: detector ID

	return;
	
}

void MiniballEventBuilder::BeamDumpFinder(){

	// Build individual beam dump events
	// Loop over all the events in beam dump detectors
	for( unsigned int i = 0; i < bd_en_list.size(); ++i ) {
	
		bd_evt->SetEnergy( bd_en_list.at(i) );
		bd_evt->SetTime( bd_ts_list.at(i) );
		bd_evt->SetDetector( bd_det_list.at(i) );
		write_evts->AddEvt( bd_evt );
		bd_ctr++;
		
	}
	
	return;
	
}

void MiniballEventBuilder::SpedeFinder(){

	// Build individual Spede events
	// Loop over all the events in Spede detector
	for( unsigned int i = 0; i < spede_en_list.size(); ++i ) {
	
		spede_evt->SetEnergy( spede_en_list.at(i) );
		spede_evt->SetTime( spede_ts_list.at(i) );
		spede_evt->SetSegment( spede_seg_list.at(i) );
		write_evts->AddEvt( spede_evt );
		spede_ctr++;

	}

	return;
	
}

void MiniballEventBuilder::IonChamberFinder(){

	// Build individual ion chamber events
	// Checks to prevent re-using events
	std::vector<unsigned int> index;
	std::vector<unsigned int> layer;
	bool flag_skip;
	
	// Loop over IonChamber events
	for( unsigned int i = 0; i < ic_en_list.size(); ++i ) {

		ic_evt->ClearEvt();

		if( ic_id_list[i] == 0 ){
			
			ic_evt->SetdETime( ic_ts_list[i] );
			ic_dE->Fill( ic_en_list[i] );
	
		}
	
		else if( ic_id_list[i] == set->GetNumberOfIonChamberLayers()-1 ){
		
			ic_evt->SetETime( ic_ts_list[i] );
			ic_E->Fill( ic_en_list[i] );
	
		}

		ic_evt->AddIonChamber( ic_en_list[i], ic_id_list[i] );
		index.push_back( i );
		layer.push_back( ic_id_list[i] );
		
		// Look for matching events in other layers
		for( unsigned int j = 0; j < ic_en_list.size(); ++j ) {

			// Can't be coincident with itself
			if( i == j ) continue;

			// Time difference plot
			ic_td->Fill( (double)ic_ts_list[i] - (double)ic_ts_list[j] );
			
			// Check if we already used this hit
			flag_skip = false;
			for( unsigned int k = 0; k < index.size(); ++k ) {
				if( index[k] == j ) flag_skip = true;
				if( layer[k] == ic_id_list[j] ) flag_skip = true;
			}
			
			// Found a match
			if( ic_id_list[j] != ic_id_list[i] && !flag_skip &&
			   TMath::Abs( (double)ic_ts_list[i] - (double)ic_ts_list[j] ) < set->GetIonChamberHitWindow() ){
				
				index.push_back( j );
				layer.push_back( ic_id_list[j] );
				ic_evt->AddIonChamber( ic_en_list[j], ic_id_list[j] );
				
				if( ic_id_list[j] == 0 )
					ic_evt->SetdETime( ic_ts_list[j] );
				else if( ic_id_list[j] == set->GetNumberOfIonChamberLayers()-1 )
					ic_evt->SetETime( ic_ts_list[j] );

			}
			
		}
		
		// Histogram the Ionchamber
		ic_dE_E->Fill( ic_evt->GetEnergyRest(), ic_evt->GetEnergyLoss() );

		// Fill the tree and get ready for next ion chamber event
		write_evts->AddEvt( ic_evt );
		ic_ctr++;
					
	}
	

	
	return;
	
}



unsigned long MiniballEventBuilder::BuildEvents() {
	
	/// Function to loop over the sort tree and build array and recoil events

	// Load the full tree if possible
	//output_tree->SetMaxVirtualSize(200e6);	// 200 MB
	//input_tree->SetMaxVirtualSize(200e6); 	// 200 MB
	//input_tree->LoadBaskets(200e6); 		// Load 200 MB of data to memory

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " Event Building: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;

	std::cout << "\tnumber of MBS Events/triggers in input tree = ";
	std::cout << mbsinfo_tree->GetEntries() << std::endl;
	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {
		
		// Current event data
		//if( input_tree->MemoryFull(30e6) )
		//	input_tree->DropBaskets();

		// First event, yes please!
		if( i == 0 ){

			input_tree->GetEntry(i);
			myeventid = in_data->GetEventID();
			myeventtime = in_data->GetTime();

			// Try to get the MBS info event with the index
			if( mbsinfo_tree->GetEntryWithIndex( myeventid ) < 0 ) {

				// Look for the matches MBS Info event if we didn't match automatically
				for( long j = 0; j < mbsinfo_tree->GetEntries(); ++j ){

					mbsinfo_tree->GetEntry(j);
					if( mbs_info->GetEventID() == myeventid ) {
						myeventtime = mbs_info->GetTime();
						break;
					}

					// Panic if we failed!
					if( j+1 == mbsinfo_tree->GetEntries() ) {
						std::cerr << "Didn't find matching MBS Event IDs at start of the file: ";
						std::cerr << myeventid << std::endl;
					}

				}

			}

			std::cout << "MBS Trigger time = " << myeventtime << std::endl;

		}

		// Get the time of the event
		mytime = in_data->GetTime(); // this is normal
		//myhittime = in_data->GetTime();	// this is for is697
		//mytime = myeventtime + myhittime; // this is for is697
		
		// check time stamp monotonically increases!
		if( time_prev > mytime ) {
			
			std::cout << "Out of order event in file ";
			std::cout << input_tree->GetName() << std::endl;
			
		}
			
		// check event id is increasing
		//if( preveventid > myeventid ) {

		//	std::cout << "Out of order event " << myeventid;
		//	std::cout << " < " << preveventid << std::endl;

		//}

		// record time of this event
		time_prev = mytime;
		
		// assume this is above threshold initially
		mythres = true;

		// ------------------------------------------ //
		// Find FEBEX data
		// ------------------------------------------ //
		if( in_data->IsFebex() ) {
			
			// Get the data
			febex_data = in_data->GetFebexData();
			mysfp = febex_data->GetSfp();
			myboard = febex_data->GetBoard();
			mych = febex_data->GetChannel();
			if( overwrite_cal ) {
				
				myenergy = cal->FebexEnergy( mysfp, myboard, mych,
									febex_data->GetQint() );
				
				if( febex_data->GetQint() > cal->FebexThreshold( mysfp, myboard, mych ) )
					mythres = true;
				else mythres = false;

			}
			
			else {
				
				myenergy = febex_data->GetEnergy();
				mythres = febex_data->IsOverThreshold();

			}
			
			// Increment event counters
			n_febex_data++;
			n_sfp[mysfp]++;
			n_board[mysfp][myboard]++;
			
			// Is it a gamma ray from Miniball?
			if( set->IsMiniball( mysfp, myboard, mych ) && mythres ) {
				
				// Increment counts and open the event
				n_miniball++;
				hit_ctr++;
				event_open = true;
				
				mb_en_list.push_back( myenergy );
				mb_ts_list.push_back( mytime );
				mb_clu_list.push_back( set->GetMiniballCluster( mysfp, myboard, mych ) );
				mb_cry_list.push_back( set->GetMiniballCrystal( mysfp, myboard, mych ) );
				mb_seg_list.push_back( set->GetMiniballSegment( mysfp, myboard, mych ) );
				
			}
			
			// Is it a particle from the CD?
			else if( set->IsCD( mysfp, myboard, mych ) && mythres ) {
				
				// Increment counts and open the event
				n_cd++;
				hit_ctr++;
				event_open = true;
				
				cd_en_list.push_back( myenergy );
				cd_ts_list.push_back( mytime );
				cd_det_list.push_back( set->GetCDDetector( mysfp, myboard, mych ) );
				cd_sec_list.push_back( set->GetCDSector( mysfp, myboard, mych ) );
				cd_side_list.push_back( set->GetCDSide( mysfp, myboard, mych ) );
				cd_strip_list.push_back( set->GetCDStrip( mysfp, myboard, mych ) );
				
			}
			
			// Is it an electron from Spede?
			else if( set->IsSpede( mysfp, myboard, mych ) && mythres ) {
				
				// Increment counts and open the event
				n_spede++;
				hit_ctr++;
				event_open = true;
				
				spede_en_list.push_back( myenergy );
				spede_ts_list.push_back( mytime );
				spede_seg_list.push_back( set->GetSpedeSegment( mysfp, myboard, mych ) );
				
			}
			
			// Is it a gamma ray from the beam dump?
			else if( set->IsBeamDump( mysfp, myboard, mych ) && mythres ) {
				
				// Increment counts and open the event
				n_bd++;
				hit_ctr++;
				event_open = true;
				
				bd_en_list.push_back( myenergy );
				bd_ts_list.push_back( mytime );
				bd_det_list.push_back( set->GetBeamDumpDetector( mysfp, myboard, mych ) );
				
			}
			
			// Is it an IonChamber event
			else if( set->IsIonChamber( mysfp, myboard, mych ) && mythres ) {
				
				// Increment counts and open the event
				n_ic++;
				hit_ctr++;
				event_open = true;
				
				ic_en_list.push_back( myenergy );
				ic_ts_list.push_back( mytime );
				ic_id_list.push_back( set->GetIonChamberLayer( mysfp, myboard, mych ) );
				
			}

			
			// Is it the start event?
			if( febex_time_start.at( mysfp ).at( myboard ) == 0 )
				febex_time_start.at( mysfp ).at( myboard ) = mytime;
			
			// or is it the end event (we don't know so keep updating)
			febex_time_stop.at( mysfp ).at( myboard ) = mytime;

		}
		
		
		// ------------------------------------------ //
		// Find info events, like timestamps etc
		// ------------------------------------------ //
		else if( in_data->IsInfo() ) {
			
			// Increment event counter
			n_info_data++;
			
			info_data = in_data->GetInfoData();
			
			// Update EBIS time
			if( info_data->GetCode() == set->GetEBISCode() &&
				TMath::Abs( (double)ebis_time - (double)info_data->GetTime() ) > 1e3 ) {
				
				ebis_time = info_data->GetTime();
				ebis_T = (double)ebis_time - (double)ebis_prev;
				ebis_f = 1e9 / ebis_T;
				if( ebis_prev != 0 ) {
					ebis_period->Fill( ebis_T );
					ebis_freq->Fill( ebis_time, ebis_f );
				}
				ebis_prev = ebis_time;
				n_ebis++;
				
			} // EBIS code
		
			// Update T1 time
			if( info_data->GetCode() == set->GetT1Code() &&
				TMath::Abs( (double)t1_time - (double)info_data->GetTime() ) > 1e3 ){
				
				t1_time = info_data->GetTime();
				t1_T = (double)t1_time - (double)t1_prev;
				t1_f = 1e9 / t1_T;
				if( t1_prev != 0 ) {
					t1_period->Fill( t1_T );
					t1_freq->Fill( t1_time, t1_f );
				}
				t1_prev = t1_time;
				n_t1++;

			} // T1 code
			
			// Update SuperCycle time
			if( info_data->GetCode() == set->GetSCCode() &&
				TMath::Abs( (double)sc_time - (double)info_data->GetTime() ) > 1e3 ){
				
				sc_time = info_data->GetTime();
				sc_T = (double)sc_time - (double)sc_prev;
				sc_f = 1e9 / sc_T;
				if( sc_prev != 0 ) {
					sc_period->Fill( sc_T );
					sc_freq->Fill( sc_time, sc_f );
				}
				sc_prev = sc_time;
				n_sc++;

			} // SuperCycle code
			
			// Update pulser time
			if( info_data->GetCode() == set->GetPulserCode() ) {
				
				pulser_time = info_data->GetTime();
				pulser_T = (double)pulser_time - (double)pulser_prev;
				pulser_f = 1e9 / pulser_T;
				if( pulser_prev != 0 ) {
					pulser_period->Fill( pulser_T );
					pulser_freq->Fill( pulser_time, pulser_f );
				}
				pulser_prev = pulser_time;
				n_pulser++;

			} // pulser code

			// Check the pause events for each module
			if( info_data->GetCode() == set->GetPauseCode() ) {
				
				if( info_data->GetSfp() < set->GetNumberOfFebexSfps() &&
				    info_data->GetBoard() < set->GetNumberOfFebexBoards() ) {

					n_pause[info_data->GetSfp()][info_data->GetBoard()]++;
					flag_pause[info_data->GetSfp()][info_data->GetBoard()] = true;
					pause_time[info_data->GetSfp()][info_data->GetBoard()] = info_data->GetTime();
				
				}
				
				else {
					
					std::cerr << "Bad pause event in SFP " << (int)info_data->GetSfp();
					std::cerr << ", board " << (int)info_data->GetBoard() << std::endl;
				
				}

			} // pause code
			
			// Check the resume events for each module
			if( info_data->GetCode() == set->GetResumeCode() ) {
				
				if( info_data->GetSfp() < set->GetNumberOfFebexSfps() &&
				    info_data->GetBoard() < set->GetNumberOfFebexBoards() ) {
				
					n_resume[info_data->GetSfp()][info_data->GetBoard()]++;
					flag_resume[info_data->GetSfp()][info_data->GetBoard()] = true;
					resume_time[info_data->GetSfp()][info_data->GetBoard()] = info_data->GetTime();
					
					// Work out the dead time
					febex_dead_time[info_data->GetSfp()][info_data->GetBoard()] += resume_time[info_data->GetSfp()][info_data->GetBoard()];
					febex_dead_time[info_data->GetSfp()][info_data->GetBoard()] -= pause_time[info_data->GetSfp()][info_data->GetBoard()];

					// If we have didn't get the pause, module was stuck at start of run
					if( !flag_pause[info_data->GetSfp()][info_data->GetBoard()] ) {

						std::cout << "SFP " << info_data->GetSfp();
						std::cout << ", board " << info_data->GetBoard();
						std::cout << " was blocked at start of run for ";
						std::cout << (double)resume_time[info_data->GetSfp()][info_data->GetBoard()]/1e9;
						std::cout << " seconds" << std::endl;
					
					}
				
				}
				
				else {
					
					std::cerr << "Bad resume event in SFP " << (int)info_data->GetSfp();
					std::cerr << ", board " << (int)info_data->GetBoard() << std::endl;
				
				}
				
			} // resume code
			
			// Now reset previous timestamps
			if( info_data->GetCode() == set->GetPulserCode() )
				pulser_prev = pulser_time;

						
		} // is info data

		// Sort out the timing for the event window
		// but only if it isn't an info event, i.e only for real data
		if( !in_data->IsInfo() ) {
			
			// if this is first datum included in Event
			if( hit_ctr == 1 && mythres ) {
				
				time_min	= mytime;
				time_max	= mytime;
				time_first	= mytime;
				
			}
			
			// Update min and max
			if( mytime > time_max ) time_max = mytime;
			else if( mytime < time_min ) time_min = mytime;
			
		} // not info data

		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		
		if( input_tree->GetEntry(i+1) ) {
			
			// Get the next MBS event ID
			preveventid = myeventid;
			myeventid = in_data->GetEventID();

			// If the next MBS event ID is the same, carry on
			// If not, we have to go look for the next trigger time
			if( myeventid != preveventid ) {

				// Close the event
				flag_close_event = true;

				// And find the next MBS event ID
				if( mbsinfo_tree->GetEntryWithIndex( myeventid ) < 0 ) {

					std::cerr << "MBS Event " << myeventid << " not found by index, looking up manually" << std::endl;

					// Look for the matches MBS Info event if we didn't match automatically
					for( long j = 0; j < mbsinfo_tree->GetEntries(); ++j ){

						mbsinfo_tree->GetEntry(j);
						if( mbs_info->GetEventID() == myeventid ) {
							myeventtime = mbs_info->GetTime();
							break;
						}

						// Panic if we failed!
						if( j+1 == mbsinfo_tree->GetEntries() ) {
							std::cerr << "Didn't find matching MBS Event IDs at start of the file: ";
							std::cerr << myeventid << std::endl;
						}
					}

				}

				else myeventtime = mbs_info->GetTime();

			}

			// BELOW IS THE TIME-ORDERED METHOD!

			// Get next time
			//myhittime = in_data->GetTime();
			//mytime = myhittime + myeventtime;
			mytime = in_data->GetTime();
			time_diff = mytime - time_first;

			// window = time_stamp_first + time_window
			if( time_diff > build_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event
				
			// Fill tdiff hist only for real data
			if( !in_data->IsInfo() ) {
				
				tdiff->Fill( time_diff );
				if( !mythres )
					tdiff_clean->Fill( time_diff );
			
			}

		} // if next entry beyond time window: close event!
		
		
		//----------------------------
		// if close this event or last entry
		//----------------------------
		if( flag_close_event || (i+1) == n_entries ) {

			// If we opened the event, then sort it out
			if( event_open ) {
			
				//----------------------------------
				// Build array events, recoils, etc
				//----------------------------------
				GammaRayFinder();		// perform addback
				ParticleFinder();		// sort out CD n/p correlations
				BeamDumpFinder();		// sort out beam dump events
				SpedeFinder();			// sort out Spede events
				IonChamberFinder();		// sort out beam dump events

				// ------------------------------------
				// Add timing and fill the ISSEvts tree
				// ------------------------------------
				write_evts->SetEBIS( ebis_time );
				write_evts->SetT1( t1_time );
				write_evts->SetSC( sc_time );
				if( write_evts->GetGammaRayMultiplicity() ||
					write_evts->GetGammaRayAddbackMultiplicity() ||
					write_evts->GetParticleMultiplicity() ||
				    write_evts->GetSpedeMultiplicity() ||
				    write_evts->GetIonChamberMultiplicity() ||
				    write_evts->GetBeamDumpMultiplicity() )
					output_tree->Fill();


				// Clean up if the next event is going to make the tree full
				//if( output_tree->MemoryFull(30e6) )
				//	output_tree->DropBaskets();

			}
			
			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			Initialise();
			
		} // if close event && hit_ctr > 0
		
		// Progress bar
		bool update_progress = false;
		if( n_entries < 200 )
			update_progress = true;
		else if( i % (n_entries/100) == 0 || i+1 == n_entries )
			update_progress = true;
		
		if( update_progress ) {

			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_entries;

			// Progress bar in GUI
			if( _prog_ ) {
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}

			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();

		}		
		
	} // End of main loop over TTree to process raw FEBEX data entries (for n_entries)
	
	//--------------------------
	// Clean up
	//--------------------------

	std::stringstream ss_log;
	ss_log << "\n MiniballEventBuilder finished..." << std::endl;
	ss_log << "  FEBEX data packets = " << n_febex_data << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
		ss_log << "   SFP " << i << " events = " << n_sfp[i] << std::endl;
		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
			ss_log << "    Board " << j << " events = " << n_board[i][j] << std::endl;
	//		ss_log << "             pause = " << n_pause[i][j] << std::endl;
	//		ss_log << "            resume = " << n_resume[i][j] << std::endl;
	//		ss_log << "         dead time = " << (double)febex_dead_time[i][j]/1e9 << " s" << std::endl;
			ss_log << "          run time = " << (double)(febex_time_stop[i][j]-febex_time_start[i][j])/1e9 << " s" << std::endl;
		}
	}
	ss_log << "  Info data packets = " << n_info_data << std::endl;
	ss_log << "   Pulser events = " << n_pulser << std::endl;
	ss_log << "   EBIS events = " << n_ebis << std::endl;
	ss_log << "   T1 events = " << n_t1 << std::endl;
	ss_log << "   SuperCycle events = " << n_sc << std::endl;
	ss_log << "  Tree entries = " << output_tree->GetEntries() << std::endl;
	ss_log << "   Miniball triggers = " << n_miniball << std::endl;
	ss_log << "    Gamma singles events = " << gamma_ctr << std::endl;
	ss_log << "    Gamma addback events = " << gamma_ab_ctr << std::endl;
	ss_log << "   CD detector triggers = " << n_cd << std::endl;
	ss_log << "    Particle events = " << cd_ctr << std::endl;
	ss_log << "   SPEDE triggers = " << n_spede << std::endl;
	ss_log << "    Electron events = " << spede_ctr << std::endl;
	ss_log << "   Beam dump triggers = " << n_bd << std::endl;
	ss_log << "    Beam dump gamma events = " << bd_ctr << std::endl;
	ss_log << "   IonChamber triggers = " << n_ic << std::endl;
	ss_log << "    IonChamber ion events = " << ic_ctr << std::endl;

	std::cout << ss_log.str();
	if( log_file.is_open() && flag_input_file ) log_file << ss_log.str();

	std::cout << "Writing output file...\r";
	std::cout.flush();
	output_file->Write( 0, TObject::kWriteDelete );
	
	std::cout << "Writing output file... Done!" << std::endl << std::endl;

	return n_entries;
	
}
