#ifndef __SETTINGS_HH
#define __SETTINGS_HH

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include "TObject.h"
#include "TSystem.h"
#include "TEnv.h"

/// A class to read in the settings file in ROOT's TConfig format.
/// This has the number of modules, channels and things
/// It also defines which detectors are which

class MiniballSettings : public TObject {

public:

	MiniballSettings();
	MiniballSettings( std::string filename );
	~MiniballSettings() {};
	
	void ReadSettings();
	void PrintSettings();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
		
	// FEBEX settings
	inline unsigned int GetNumberOfFebexSfps(){ return n_febex_sfp; };
	inline unsigned int GetNumberOfFebexBoards(){ return n_febex_board; };
	inline unsigned int GetNumberOfFebexChannels(){ return n_febex_ch; };
	
	// Info settings
	inline unsigned int GetSyncCode(){ return sync_code; };
	inline unsigned int GetTimestampCode(){ return thsb_code; };

	inline unsigned int GetPauseCode(){ return pause_code;};
	inline unsigned int GetResumeCode(){ return resume_code;};

	inline unsigned int GetPulserSfp(){ return pulser_sfp; };
	inline unsigned int GetPulserBoard(){ return pulser_board; };
	inline unsigned int GetPulserChannel(){ return pulser_ch; };
	inline unsigned int GetPulserCode(){ return pulser_code; };
	
	inline unsigned int GetEBISSfp(){ return ebis_sfp; };
	inline unsigned int GetEBISBoard(){ return ebis_board; };
	inline unsigned int GetEBISChannel(){ return ebis_ch; };
	inline unsigned int GetEBISCode(){ return ebis_code; };

	inline unsigned int GetT1Sfp(){ return t1_sfp; };
	inline unsigned int GetT1Board(){ return t1_board; };
	inline unsigned int GetT1Channel(){ return t1_ch; };
	inline unsigned int GetT1Code(){ return t1_code; };

	inline unsigned int GetSCSfp(){ return sc_sfp; };
	inline unsigned int GetSCBoard(){ return sc_board; };
	inline unsigned int GetSCChannel(){ return sc_ch; };
	inline unsigned int GetSCCode(){ return sc_code; };


	// Event builder
	inline double GetEventWindow(){ return event_window; };
	
	
	// Data settings
	void SetBlockSize( unsigned int size ){ block_size = size; };
	inline unsigned int GetBlockSize(){ return block_size; };
	inline unsigned int IsFebexOnly(){ return flag_febex_only; };


	// Miniball array
	inline unsigned int GetNumberOfMiniballClusters(){ return n_mb_cluster; };
	inline unsigned int GetNumberOfMiniballCrystals(){ return n_mb_crystal; };
	inline unsigned int GetNumberOfMiniballSegments(){ return n_mb_segment; };
	bool IsMiniball( unsigned int sfp, unsigned int board, unsigned int ch );
	int GetMiniballID( unsigned int sfp, unsigned int board, unsigned int ch,
					  std::vector<std::vector<std::vector<int>>> vector );
	inline int GetMiniballCluster( unsigned int sfp, unsigned int board, unsigned int ch ){
		return GetMiniballID( sfp, board, ch, mb_cluster );
	};
	inline int GetMiniballCrystal( unsigned int sfp, unsigned int board, unsigned int ch ){
		return GetMiniballID( sfp, board, ch, mb_crystal );
	};
	inline int GetMiniballSegment( unsigned int sfp, unsigned int board, unsigned int ch ){
		return GetMiniballID( sfp, board, ch, mb_segment );
	};
	inline double GetMiniballCrystalHitWindow(){ return mb_hit_window; };
	inline double GetMiniballAddbackHitWindow(){ return mb_hit_window; };


	// CD detector
	inline unsigned int GetNumberOfCDDetectors(){ return n_cd_det; };
	inline unsigned int GetNumberOfCDSectors(){ return n_cd_sector; };
	inline unsigned int GetNumberOfCDSides(){ return n_cd_side; };
	inline unsigned int GetNumberOfCDPStrips(){ return n_cd_pstrip; };
	inline unsigned int GetNumberOfCDNStrips(){ return n_cd_nstrip; };
	bool IsCD( unsigned int sfp, unsigned int board, unsigned int ch );
	int GetCDID( unsigned int sfp, unsigned int board, unsigned int ch,
					  std::vector<std::vector<std::vector<int>>> vector );
	inline int GetCDDetector( unsigned int sfp, unsigned int board, unsigned int ch ){
		return GetCDID( sfp, board, ch, cd_det );
	};
	inline int GetCDSector( unsigned int sfp, unsigned int board, unsigned int ch ){
		return GetCDID( sfp, board, ch, cd_sector );
	};
	inline int GetCDSide( unsigned int sfp, unsigned int board, unsigned int ch ){
		return GetCDID( sfp, board, ch, cd_side );
	};
	inline int GetCDStrip( unsigned int sfp, unsigned int board, unsigned int ch ){
		return GetCDID( sfp, board, ch, cd_strip );
	};
	inline double GetCDHitWindow(){ return cd_hit_window; };

	
	// Beam dump detector
	inline unsigned int GetNumberOfBeamDumpDetectors(){ return n_bd_det; };
	bool IsBeamDump( unsigned int sfp, unsigned int board, unsigned int ch );
	int GetBeamDumpDetector( unsigned int sfp, unsigned int board, unsigned int ch );

	
	// SPEDE detector
	inline unsigned int GetNumberOfSpedeSegments(){ return n_spede_seg; };
	bool IsSpede( unsigned int sfp, unsigned int board, unsigned int ch );
	int GetSpedeSegment( unsigned int sfp, unsigned int board, unsigned int ch );

	
	// IonChamber
	inline unsigned int GetNumberOfIonChamberLayers(){ return n_ic_layer; };
	bool IsIonChamber( unsigned int sfp, unsigned int board, unsigned int ch );
	int GetIonChamberLayer( unsigned int sfp, unsigned int board, unsigned int ch );
	inline double GetIonChamberHitWindow(){ return ic_hit_window; };
	
	ClassDef( MiniballSettings, 1 )

private:

	std::string fInputFile;

	// FEBEX settings
	unsigned int n_febex_sfp;		///< Number of SFPs in acquisition
	unsigned int n_febex_board;		///< Maximum number of boards per SFP
	unsigned int n_febex_ch;		///< Number of channels per board (16 for FEBEX4)
	
	
	// Miniball array settings
	unsigned int n_mb_cluster;		///< Miniball usuall has 8 triple cluster detectors
	unsigned int n_mb_crystal;		///< Number of crystals in a triple cluster is 3
	unsigned int n_mb_segment;		///< Number of sgements in a crystal is 7, including core = 0
	
	// Miniball array electronics mapping
	std::vector<std::vector<std::vector<unsigned int>>> mb_sfp;		///< A list of SFP numbers for each CD detector element and sector
	std::vector<std::vector<std::vector<unsigned int>>> mb_board;	///< A list of board numbers for each CD detector element and sector
	std::vector<std::vector<std::vector<unsigned int>>> mb_ch;		///< A list of channel numbers for each CD detector element and sector
	std::vector<std::vector<std::vector<int>>> mb_cluster;			///< A channel map for the Miniball cluster IDs (-1 if not Miniball)
	std::vector<std::vector<std::vector<int>>> mb_crystal;			///< A channel map for the Miniball crystal IDs (-1 if not Miniball)
	std::vector<std::vector<std::vector<int>>> mb_segment;			///< A channel map for the Miniball segment IDs (-1 if not Miniball)

	// CD settings
	unsigned int n_cd_det;			///< Number of CD detectors (just 1 for Coulex)
	unsigned int n_cd_sector;		///< number of sectors in the CD (or quadrants, it's 4)
	unsigned int n_cd_side;			///< number of sides, it's always 2, i.e. p-side and n-side
	unsigned int n_cd_pstrip;		///< number of p-side strips
	unsigned int n_cd_nstrip;		///< number of n-side strips

	// CD electronics mapping
	std::vector<std::vector<std::vector<std::vector<unsigned int>>>> cd_sfp;	///< A list of SFP numbers for each CD detector, sector, side and strip
	std::vector<std::vector<std::vector<std::vector<unsigned int>>>> cd_board;	///< A list of board numbers for each CD detector, sector, side and strip
	std::vector<std::vector<std::vector<std::vector<unsigned int>>>> cd_ch;		///< A list of channel numbers for each CD detector, sector, side and strip
	std::vector<std::vector<std::vector<int>>> cd_det;							///< A channel map for the CD detectors (-1 if not a CD, 0 forward CD, 1 backward CD)
	std::vector<std::vector<std::vector<int>>> cd_sector;						///< A channel map for the CD sectors (-1 if not a CD, 0-3 for quadrants)
	std::vector<std::vector<std::vector<int>>> cd_side;							///< A channel map for the CD sides (-1 if not a CD, 0 for p-side, 1 for n-side)
	std::vector<std::vector<std::vector<int>>> cd_strip;						///< A channel map for the CD strips IDs (-1 if not a CD, strip ID otherwise)

	// Beam dump settings
	unsigned int n_bd_det;		///< Number of beam dump detectors

	// Beam dump electronics mapping
	std::vector<unsigned int> bd_sfp;						///< A list of SFP numbers for each beam dump detector
	std::vector<unsigned int> bd_board;						///< A list of board numbers for each beam dump detector
	std::vector<unsigned int> bd_ch;						///< A list of channel numbers for each beam dump detector
	std::vector<std::vector<std::vector<int>>> bd_det;		///< A channel map for the beam dump detectors (-1 if not a beam dump, otherwise detector number)

	// SPEDE settings
	unsigned int n_spede_seg;								///< Number of SPEDE segments, usually 24

	// SPEDE electronics mapping
	std::vector<unsigned int> spede_sfp;					///< A list of SFP numbers for each SPEDE segment
	std::vector<unsigned int> spede_board;					///< A list of board numbers for each SPEDE segment
	std::vector<unsigned int> spede_ch;						///< A list of channel numbers for each SPEDE segment
	std::vector<std::vector<std::vector<int>>> spede_seg;	///< A channel map for the SPEDE segments (-1 if not a SPEDE, otherwise segment number)

	// IonChamber settings
	unsigned int n_ic_layer;								///< Number of IonChamber layers, usually 2, dE (gas) and E (Si)

	// SPEDE electronics mapping
	std::vector<unsigned int> ic_sfp;						///< A list of SFP numbers for each IonChamber segment
	std::vector<unsigned int> ic_board;						///< A list of board numbers for each IonChamber segment
	std::vector<unsigned int> ic_ch;						///< A list of channel numbers for each IonChamber segment
	std::vector<std::vector<std::vector<int>>> ic_layer;	///< A channel map for the IonChamber segments (-1 if not a IonChamber, otherwise layer number, i.e dE (gas) = 0, E (Si) = 1)


	// Info code settings
	unsigned int sync_code;				///< Medium significant bits of the timestamp are here
	unsigned int thsb_code;				///< Highest significant bits of the timestamp are here
	unsigned int pause_code;        	///< Info code when acquisition has paused due to a full buffer
	unsigned int resume_code;       	///< Info code when acquisition has resumed after a pause.
	unsigned int pulser_sfp;			///< Location of the pulser in the FEBEX system (sfp)
	unsigned int pulser_board;			///< Location of the pulser in the FEBEX system (board)
	unsigned int pulser_ch;				///< Location of the pulser in the FEBEX system (channel)
	unsigned int pulser_code;			///< Info code when we have a pulser event in InfoData packets
	unsigned int ebis_sfp;				///< Location of the EBIS signal in the FEBEX system (sfp)
	unsigned int ebis_board;			///< Location of the EBIS signal in the FEBEX system (board)
	unsigned int ebis_ch;				///< Location of the EBIS signal in the FEBEX system (channel)
	unsigned int ebis_code;				///< Info code when we have an EBIS event in InfoData packets
	unsigned int t1_sfp;				///< Location of the T1 signal in the FEBEX system (sfp)
	unsigned int t1_board;				///< Location of the T1 signal in the FEBEX system (board)
	unsigned int t1_ch;					///< Location of the T1 signal in the FEBEX system (channel)
	unsigned int t1_code;				///< Info code when we have a T1 event in InfoData packets
	unsigned int sc_sfp;				///< Location of the SuperCycle signal in the FEBEX system (sfp)
	unsigned int sc_board;				///< Location of the SuperCycle signal in the FEBEX system (board)
	unsigned int sc_ch;					///< Location of the SuperCycle signal in the FEBEX system (channel)
	unsigned int sc_code;				///< Info code when we have a SuperCycle event in InfoData packets

	
	// Event builder
	double event_window;			///< Event builder time window in ns
	
	// Hit windows for complex events
	double mb_hit_window;			///< Prompt time for correlated Miniball events in crystal, i.e. segmen-core events
	double ab_hit_window;			///< Prompt time for correlated Miniball events in cluster, i.e. addback events
	double cd_hit_window;			///< Prompt time for correlated CD events
	double ic_hit_window;			///< Prompt time for correlated ionchamber events

	// Data format
	unsigned int block_size;		///< not yet implemented, needs C++ style reading of data files
	bool flag_febex_only;			///< when there is only FEBEX data in the file

	
};

#endif
