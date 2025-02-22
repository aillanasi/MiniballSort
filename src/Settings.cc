#include "Settings.hh"

ClassImp(MiniballSettings)

MiniballSettings::MiniballSettings() {
	
	SetFile( "dummy" );
	ReadSettings();
	
}

MiniballSettings::MiniballSettings( std::string filename ) {
	
	SetFile( filename );
	ReadSettings();
	
}

void MiniballSettings::ReadSettings() {
	
	TEnv *config = new TEnv( fInputFile.data() );
	
	// FEBEX initialisation
	n_febex_sfp		= config->GetValue( "NumberOfFebexSfps", 2 );
	n_febex_board	= config->GetValue( "NumberOfFebexBoards", 12 );
	n_febex_ch		= config->GetValue( "NumberOfFebexChannels", 16 );
	
	// Miniball array initialisation
	n_mb_cluster	= config->GetValue( "NumberOfMiniballClusters", 8 );
	n_mb_crystal	= config->GetValue( "NumberOfMiniballCrystals", 3 );
	n_mb_segment	= config->GetValue( "NumberOfMiniballSegments", 7 );
	
	// CD detector initialisation
	n_cd_det		= config->GetValue( "NumberOfCDDetectors", 1 );
	n_cd_sector		= config->GetValue( "NumberOfCDSectors", 4 );
	n_cd_side		= 2; // always 2, you cannot change this!
	n_cd_pstrip		= config->GetValue( "NumberOfCDStrips.P", 16 );
	n_cd_nstrip		= config->GetValue( "NumberOfCDStrips.N", 12 );

	// Beam dump initialisation
	n_bd_det		= config->GetValue( "NumberOfBeamDumpDetectors", 1 );
	
	// SPEDE initialisation
	n_spede_seg		= config->GetValue( "NumberOfSpedeSegments", 24 );
	
	// IonChamber initialisation
	n_ic_layer		= config->GetValue( "NumberOfIonChamberLayers", 2 );
	
	// Info code initialisation
	pause_code		= 2;
	resume_code		= 3;
	sync_code		= 4;
	thsb_code		= 5;
	pulser_sfp		= config->GetValue( "Pulser.Sfp", 1 );
	pulser_board	= config->GetValue( "Pulser.Board", 8 );
	pulser_ch		= config->GetValue( "Pulser.Channel", 12 );
	pulser_code		= 20;
	ebis_sfp		= config->GetValue( "EBIS.Sfp", 1 );
	ebis_board		= config->GetValue( "EBIS.Board", 8 );
	ebis_ch			= config->GetValue( "EBIS.Channel", 11 );
	ebis_code		= 21;
	t1_sfp			= config->GetValue( "T1.Sfp", 1 );
	t1_board		= config->GetValue( "T1.Board", 8 );
	t1_ch			= config->GetValue( "T1.Channel", 13 );
	t1_code			= 22;
	sc_sfp			= config->GetValue( "SC.Sfp", 1 );
	sc_board		= config->GetValue( "SC.Board", 8 );
	sc_ch			= config->GetValue( "SC.Channel", 15 );
	sc_code			= 23;

	
	// Event builder
	event_window	= config->GetValue( "EventWindow", 3e3 );

	// Hit windows for complex events
	mb_hit_window	= config->GetValue( "MiniballCrystalHitWindow", 400. );
	ab_hit_window	= config->GetValue( "MiniballAddbackHitWindow", 400. );
	cd_hit_window	= config->GetValue( "CDHitWindow", 150. );
	ic_hit_window	= config->GetValue( "IonChamberHitWindow", 500. );

	
	// Data things
	block_size			= config->GetValue( "DataBlockSize", 0x10000 );
	flag_febex_only		= config->GetValue( "FebexOnlyData", true );

	
	
	// Electronics mapping
	mb_cluster.resize( n_febex_sfp );
	mb_crystal.resize( n_febex_sfp );
	mb_segment.resize( n_febex_sfp );
	cd_det.resize( n_febex_sfp );
	cd_sector.resize( n_febex_sfp );
	cd_side.resize( n_febex_sfp );
	cd_strip.resize( n_febex_sfp );
	bd_det.resize( n_febex_sfp );
	spede_seg.resize( n_febex_sfp );
	ic_layer.resize( n_febex_sfp );

	for( unsigned int i = 0; i < n_febex_sfp; ++i ){

		mb_cluster[i].resize( n_febex_board );
		mb_crystal[i].resize( n_febex_board );
		mb_segment[i].resize( n_febex_board );
		cd_det[i].resize( n_febex_board );
		cd_sector[i].resize( n_febex_board );
		cd_side[i].resize( n_febex_board );
		cd_strip[i].resize( n_febex_board );
		bd_det[i].resize( n_febex_board );
		spede_seg[i].resize( n_febex_board );
		ic_layer[i].resize( n_febex_board );

		for( unsigned int j = 0; j < n_febex_board; ++j ){

			mb_cluster[i][j].resize( n_febex_ch );
			mb_crystal[i][j].resize( n_febex_ch );
			mb_segment[i][j].resize( n_febex_ch );
			cd_det[i][j].resize( n_febex_ch );
			cd_sector[i][j].resize( n_febex_ch );
			cd_side[i][j].resize( n_febex_ch );
			cd_strip[i][j].resize( n_febex_ch );
			bd_det[i][j].resize( n_febex_ch );
			spede_seg[i][j].resize( n_febex_ch );
			ic_layer[i][j].resize( n_febex_ch );

			for( unsigned int k = 0; k < n_febex_ch; ++k ){

				mb_cluster[i][j][k] = -1;
				mb_crystal[i][j][k] = -1;
				mb_segment[i][j][k] = -1;
				cd_det[i][j][k]     = -1;
				cd_sector[i][j][k]  = -1;
				cd_side[i][j][k]	= -1;
				cd_strip[i][j][k]	= -1;
				bd_det[i][j][k]     = -1;
				spede_seg[i][j][k]  = -1;
				ic_layer[i][j][k]   = -1;

			} // k: febex ch
			
		} // j: febex board

	} // i: febex sfp
	
	
	// Miniball array electronics mapping
	int d, s, b, c;
	mb_sfp.resize( n_mb_cluster );
	mb_board.resize( n_mb_cluster );
	mb_ch.resize( n_mb_cluster );
	
	for( unsigned int i = 0; i < n_mb_cluster; ++i ){

		mb_sfp[i].resize( n_mb_crystal );
		mb_board[i].resize( n_mb_crystal );
		mb_ch[i].resize( n_mb_crystal );

		for( unsigned int j = 0; j < n_mb_crystal; ++j ){

			mb_sfp[i][j].resize( n_mb_segment );
			mb_board[i][j].resize( n_mb_segment );
			mb_ch[i][j].resize( n_mb_segment );

			for( unsigned int k = 0; k < n_mb_segment; ++k ){

				d = i*3 + j;			// Crystal ordering: 0-23
				s = 0;					// spread 24 crystals over 1 SFPs
				b = d/2;				// 2 crystals per board
				c = k + 9*(d&0x1);		// odd crystals starts at ch9
				mb_sfp[i][j][k]		= config->GetValue( Form( "Miniball_%d_%d_%d.Sfp", i, j, k ), s );
				mb_board[i][j][k]	= config->GetValue( Form( "Miniball_%d_%d_%d.Board", i, j, k ), b );
				mb_ch[i][j][k]		= config->GetValue( Form( "Miniball_%d_%d_%d.Channel", i, j, k ), c );

				if( mb_sfp[i][j][k] < n_febex_sfp &&
					mb_board[i][j][k] < n_febex_board &&
					mb_ch[i][j][k] < n_febex_ch ){
					
					mb_cluster[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] = i;
					mb_crystal[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] = j;
					mb_segment[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] = k;

				}
				
				else {
					
					std::cerr << "Dodgy Miniball settings: sfp = " << mb_sfp[i][j][k];
					std::cerr << ", board = " << mb_board[i][j][k];
					std::cerr << ", channel = " << mb_ch[i][j][k] << std::endl;

				}

			} // k: mb segment

		} // j: mb crystal

	} // i: mb cluster
	

	// CD detector electronics mapping
	unsigned int side_size;
	std::string side_str;
	cd_sfp.resize( n_cd_det );
	cd_board.resize( n_cd_det );
	cd_ch.resize( n_cd_det );
	
	for( unsigned int i = 0; i < n_cd_det; ++i ){
		
		cd_sfp[i].resize( n_cd_sector );
		cd_board[i].resize( n_cd_sector );
		cd_ch[i].resize( n_cd_sector );
		
		for( unsigned int j = 0; j < n_cd_sector; ++j ){
			
			cd_sfp[i][j].resize( n_cd_side );
			cd_board[i][j].resize( n_cd_side );
			cd_ch[i][j].resize( n_cd_side );
			
			for( unsigned int k = 0; k < n_cd_side; ++k ){
				
				// p or n side?
				if( k == 0 ) side_str = "P";
				else side_str = "N";

				// p or n side?
				if( k == 0 ) side_size = n_cd_pstrip;
				else side_size = n_cd_nstrip;

				cd_sfp[i][j][k].resize( side_size );
				cd_board[i][j][k].resize( side_size );
				cd_ch[i][j][k].resize( side_size );

				for( unsigned int l = 0; l < side_size; ++l ){
					
					s = 1;			// sfp number - all in SFP 1
					b = j*2 + k;	// boards go 0-7
					c = l;
					cd_sfp[i][j][k][l]		= config->GetValue( Form( "CD_%d_%d_%d.%s.Sfp", i, j, l, side_str.data() ), s );
					cd_board[i][j][k][l]	= config->GetValue( Form( "CD_%d_%d_%d.%s.Board", i, j, l, side_str.data() ), b );
					cd_ch[i][j][k][l]		= config->GetValue( Form( "CD_%d_%d_%d.%s.Channel", i, j, l, side_str.data() ), c );
					
					if( cd_sfp[i][j][k][l] < n_febex_sfp &&
					   cd_board[i][j][k][l] < n_febex_board &&
					   cd_ch[i][j][k][l] < n_febex_ch ){
						
						cd_det[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = i;
						cd_sector[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = j;
						cd_side[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = k;
						cd_strip[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = l;
						
					}
					
					else {
						
						std::cerr << "Dodgy CD settings: sfp = " << cd_sfp[i][j][k][l];
						std::cerr << ", board = " << cd_board[i][j][k][l];
						std::cerr << ", channel = " << cd_ch[i][j][k][l] << std::endl;
						
					}
					
				} // l: cd strips
				
			} // k: cd side
			
		} // j: cd sector
		
	} // i: cd detector
	
	
	// Beam dump detector mapping
	bd_sfp.resize( n_bd_det );
	bd_board.resize( n_bd_det );
	bd_ch.resize( n_bd_det );
	
	for( unsigned int i = 0; i < n_bd_det; ++i ){
		
		bd_sfp[i]		= config->GetValue( Form( "BeamDump_%d.Sfp", i ), 1 );
		bd_board[i]		= config->GetValue( Form( "BeamDump_%d.Board", i ), 10 );
		bd_ch[i]		= config->GetValue( Form( "BeamDump_%d.Channel", i ), (int)(i+0) );
		
		if( bd_sfp[i] < n_febex_sfp &&
		    bd_board[i] < n_febex_board &&
		    bd_ch[i] < n_febex_ch ){
			
			bd_det[bd_sfp[i]][bd_board[i]][bd_ch[i]] = i;
			
		}
		
		else {
			
			std::cerr << "Dodgy beam-dump settings: sfp = " << bd_sfp[i];
			std::cerr << ", board = " << bd_board[i];
			std::cerr << ", channel = " << bd_ch[i] << std::endl;
			
		}
		
		
	} // i: beam dump detector
	
	
	// SPEDE detector mapping
	spede_sfp.resize( n_spede_seg );
	spede_board.resize( n_spede_seg );
	spede_ch.resize( n_spede_seg );
	
	for( unsigned int i = 0; i < n_spede_seg; ++i ){
		
		s = 1;
		if( i < 16 ){
			b = 8;
			c = i;
		}
		else {
			b = 9;
			c = i-16;
		}
		
		spede_sfp[i]	= config->GetValue( Form( "Spede_%d.Sfp", i ), s );
		spede_board[i]	= config->GetValue( Form( "Spede_%d.Board", i ), b );
		spede_ch[i]		= config->GetValue( Form( "Spede_%d.Channel", i ), c );
		
		if( spede_sfp[i] < n_febex_sfp &&
		    spede_board[i] < n_febex_board &&
		    spede_ch[i] < n_febex_ch ){
			
			spede_seg[spede_sfp[i]][spede_board[i]][spede_ch[i]] = i;
			
		}
		
		else {
			
			std::cerr << "Dodgy SPEDE settings: sfp = " << spede_sfp[i];
			std::cerr << ", board = " << spede_board[i];
			std::cerr << ", channel = " << spede_ch[i] << std::endl;
			
		}
		
		
	} // i: SPEDE detector
	
	// IonChamber detector mapping
	ic_sfp.resize( n_ic_layer );
	ic_board.resize( n_ic_layer );
	ic_ch.resize( n_ic_layer );
	
	for( unsigned int i = 0; i < n_bd_det; ++i ){
		
		ic_sfp[i]		= config->GetValue( Form( "IonChamber_%d.Sfp", i ), 1 );
		ic_board[i]		= config->GetValue( Form( "IonChamber_%d.Board", i ), 10 );
		ic_ch[i]		= config->GetValue( Form( "IonChamber_%d.Channel", i ), (int)(i+0) );
		
		if( ic_sfp[i] < n_febex_sfp &&
		    ic_board[i] < n_febex_board &&
		    ic_ch[i] < n_febex_ch ){
			
			ic_layer[ic_sfp[i]][ic_board[i]][ic_ch[i]] = i;
			
		}
		
		else {
			
			std::cerr << "Dodgy IonChamber settings: sfp = " << ic_sfp[i];
			std::cerr << ", board = " << ic_board[i];
			std::cerr << ", channel = " << ic_ch[i] << std::endl;
			
		}
		
		
	} // i: IonChamber detector
	
	
	// Finished
	delete config;
	
}


bool MiniballSettings::IsMiniball( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a Miniball event
	if( mb_cluster[sfp][board][ch] >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetMiniballID( unsigned int sfp, unsigned int board, unsigned int ch,
							 std::vector<std::vector<std::vector<int>>> vector ) {
	
	/// Return the Miniball ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return vector[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad Miniball event: sfp = " << sfp;
		std::cerr << ", board = " << board << std::endl;
		std::cerr << ", channel = " << ch << std::endl;
		return -1;
		
	}
	
}


bool MiniballSettings::IsCD( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a CD event
	if( cd_det[sfp][board][ch] >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetCDID( unsigned int sfp, unsigned int board, unsigned int ch,
					  std::vector<std::vector<std::vector<int>>> vector ) {
	
	/// Return the CD ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return vector[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad CD event: sfp = " << sfp;
		std::cerr << ", board = " << board << std::endl;
		std::cerr << ", channel = " << ch << std::endl;
		return -1;
		
	}
	
}

bool MiniballSettings::IsBeamDump( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a beam dump event
	if( bd_det[sfp][board][ch] >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetBeamDumpDetector( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the beam dump detector ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return bd_det[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad beam dump event: sfp = " << sfp;
		std::cerr << ", board = " << board << std::endl;
		std::cerr << ", channel = " << ch << std::endl;
		return -1;
		
	}
	
}

bool MiniballSettings::IsSpede( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a SPEDE event
	if( spede_seg[sfp][board][ch] >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetSpedeSegment( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the SPEDE segment ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return spede_seg[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad SPEDE event: sfp = " << sfp;
		std::cerr << ", board = " << board << std::endl;
		std::cerr << ", channel = " << ch << std::endl;
		return -1;
		
	}
	
}

bool MiniballSettings::IsIonChamber( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a IonChamber event
	if( ic_layer[sfp][board][ch] >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetIonChamberLayer( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the IonChamber layer ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return ic_layer[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad IonChamber event: sfp = " << sfp;
		std::cerr << ", board = " << board << std::endl;
		std::cerr << ", channel = " << ch << std::endl;
		return -1;
		
	}
	
}
