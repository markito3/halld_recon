#ifndef DSourceComboer_h
#define DSourceComboer_h

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <stack>

#include "TF1.h"

#include "JANA/JObject.h"
#include "JANA/JEventLoop.h"

#include "particleType.h"
#include "SplitString.h"
#include "DANA/DApplication.h"
#include "HDGEOMETRY/DGeometry.h"
#include "EVENTSTORE/DESSkimData.h"

#include "PID/DNeutralShower.h"
#include "PID/DKinematicData.h"
#include "PID/DEventRFBunch.h"

#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DSourceCombo.h"
#include "ANALYSIS/DReactionStepVertexInfo.h"
#include "ANALYSIS/DReactionVertexInfo.h"
#include "DResourcePool.h"

#include "ANALYSIS/DSourceComboVertexer.h"
#include "ANALYSIS/DSourceComboP4Handler.h"
#include "ANALYSIS/DSourceComboTimeHandler.h"
#include "ANALYSIS/DParticleComboCreator.h"

using namespace std;
using namespace jana;

namespace DAnalysis
{
//finish comments in Build_ParticleCombos()
//move resource pools to kinfitter?
//change all references to bcal/fcal to z-independent/dependent showers
//When saving ROOT TTree, don't save p4 of decaying particles if mass is not constrained in kinfit!
	//And make sure it's not grabbed in DSelector by default

/****************************************************** DEFINE LAMBDAS, USING STATEMENTS *******************************************************/

struct DCompare_SourceComboInfos{
	bool operator()(const DSourceComboInfo* lhs, const DSourceComboInfo* rhs) const{return *lhs < *rhs;}
};

/********************************************************** DEFINE USING STATEMENTS ***********************************************************/

//DEFINE USING STATEMENTS
using DCombosByBeamBunch = map<vector<int>, vector<const DSourceCombo*>>;
using DSourceCombosByBeamBunchByUse = map<DSourceComboUse, DCombosByBeamBunch>;
using DComboIteratorsByBeamBunch = map<vector<int>, vector<const DSourceCombo*>::const_iterator>; //vector<int>: RF bunches (empty for all)
//The DSourceCombosByUse_Large type uses a vector to pointer so that the combos can be easily copied and reused for another use
//e.g. when you can't place a mass cut yet: 2 different uses, identical combos: far faster to just copy the pointer to the large vector
using DSourceCombosByUse_Large = map<DSourceComboUse, vector<const DSourceCombo*>*>;
using DCombosByReaction = unordered_map<const DReaction*, vector<const DParticleCombo*>>;

/************************************************************** DEFINE CLASSES ***************************************************************/

enum ComboingStage_t
{
	d_ChargedStage,
	d_MixedStage_ZIndependent,
	d_MixedStage
};

class DSourceComboer : public JObject
{
	public:

		DSourceComboer(void) = delete;
		DSourceComboer(JEventLoop* locEventLoop);
		~DSourceComboer(void);

		//RESET
		void Reset_NewEvent(JEventLoop* locEventLoop);

		//BUILD COMBOS (what should be called from the outside to do all of the work)
		DCombosByReaction Build_ParticleCombos(const DReactionVertexInfo* locReactionVertexInfo);

		//Get combo characteristics
		Charge_t Get_ChargeContent(const DSourceComboInfo* locSourceComboInfo) const{return dComboInfoChargeContent.find(locSourceComboInfo)->second;}
		bool Get_HasMassiveNeutrals(const DSourceComboInfo* locSourceComboInfo) const{return (dComboInfosWithMassiveNeutrals.find(locSourceComboInfo) != dComboInfosWithMassiveNeutrals.end());}

		//Combo utility functions
		const DSourceCombo* Get_StepSourceCombo(const DReaction* locReaction, size_t locDesiredStepIndex, const DSourceCombo* locSourceCombo_Current, size_t locCurrentStepIndex = 0) const;
		const DSourceCombo* Get_VertexPrimaryCombo(const DSourceCombo* locReactionCombo, const DReactionStepVertexInfo* locStepVertexInfo) const;
		const DSourceCombo* Get_VertexPrimaryCombo(const DSourceCombo* locReactionCombo, const DReactionStepVertexInfo* locStepVertexInfo);

		//Get combo uses
		DSourceComboUse Get_SourceComboUse(const DReactionStepVertexInfo* locStepVertexInfo) const{return dSourceComboUseReactionMap.find(locStepVertexInfo)->second;};
		DSourceComboUse Get_SourceComboUse(const DReaction* locReaction, size_t locStepIndex) const{return dSourceComboUseReactionStepMap.find(locReaction)->second.find(locStepIndex)->second;};
		DSourceComboUse Get_PrimaryComboUse(const DReactionVertexInfo* locReactionVertexInfo) const{return Get_SourceComboUse(locReactionVertexInfo->Get_StepVertexInfo(0));};

		//VERTEX-Z BINNING UTILITY FUNCTIONS
		size_t Get_PhotonVertexZBin(double locVertexZ) const;
		double Get_PhotonVertexZBinCenter(signed char locVertexZBin) const;
		size_t Get_VertexZBin_TargetCenter(void) const{return Get_PhotonVertexZBin(dTargetCenter.Z());}

		DParticleComboCreator* Get_ParticleComboCreator(void) const{return dParticleComboCreator;}

	private:

		/********************************************************** DECLARE MEMBER FUNCTIONS ***********************************************************/

		//SETUP
		void Recycle_ComboResources(DSourceCombosByUse_Large& locCombosByUse);
		void Setup_NeutralShowers(JEventLoop* locEventLoop);

		//INITIAL CHECKS
		bool Check_NumParticles(const DReaction* locReaction);
		bool Check_Skims(const DReaction* locReaction) const;

		//CREATE PHOTON COMBO INFOS & USES
		void Create_SourceComboInfos(const DReactionVertexInfo* locReactionVertexInfo);
		DSourceComboUse Create_ZDependentSourceComboUses(const DReactionVertexInfo* locReactionVertexInfo, const DSourceCombo* locReactionChargedCombo);
		DSourceComboUse Build_NewZDependentUse(const DReaction* locReaction, size_t locStepIndex, signed char locVertexZBin, const DSourceComboUse& locOrigUse, const unordered_map<size_t, DSourceComboUse>& locCreatedUseMap);

		//CREATE PHOTON COMBO INFOS & USES: UTILITY METHODS
		map<Particle_t, unsigned char> Build_ParticleMap(const DReaction* locReaction, size_t locStepIndex, Charge_t locCharge) const;
		pair<bool, map<DSourceComboUse, unsigned char>> Get_FinalStateDecayingComboUses(const DReaction* locReaction, size_t locStepIndex, const map<size_t, DSourceComboUse>& locStepComboUseMap) const;
		DSourceComboUse Make_ComboUse(Particle_t locInitPID, const map<Particle_t, unsigned char>& locNumParticles, const map<DSourceComboUse, unsigned char>& locFurtherDecays);
		const DSourceComboInfo* MakeOrGet_SourceComboInfo(const vector<pair<Particle_t, unsigned char>>& locNumParticles, const vector<pair<DSourceComboUse, unsigned char>>& locFurtherDecays);
		const DSourceComboInfo* GetOrMake_SourceComboInfo(const vector<pair<Particle_t, unsigned char>>& locNumParticles, const vector<pair<DSourceComboUse, unsigned char>>& locFurtherDecays);

		//CREATE COMBOS
		void Combo_WithNeutralsAndBeam(const vector<const DReaction*>& locReactions, const DReactionVertexInfo* locReactionVertexInfo, const DSourceComboUse& locPrimaryComboUse, const DSourceCombo* locReactionChargedCombo, const vector<int>& locBeamBunches_Charged, DCombosByReaction& locOutputComboMap);
		void Combo_WithBeam(const vector<const DReaction*>& locReactions, const DReactionVertexInfo* locReactionVertexInfo, const DSourceCombo* locReactionFullCombo, int locRFBunch, DCombosByReaction& locOutputComboMap);
		const DParticleCombo* Build_ParticleCombo(const DReactionVertexInfo* locReactionVertexInfo, const DSourceCombo* locFullCombo, const DKinematicData* locBeamParticle);

		//CREATE SOURCE COMBOS - GENERAL METHODS
		void Create_SourceCombos(const DSourceComboUse& locComboUseToCreate, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_Presiding);
		void Create_SourceCombos_Unknown(const DSourceComboUse& locComboUseToCreate, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_Presiding);

		//COMBO VERTICALLY METHODS
		//Note that vertical comboing always takes place at the same vertex-z
		void Combo_Vertically_AllDecays(const DSourceComboUse& locComboUseToCreate, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_Presiding);
		void Combo_Vertically_NDecays(const DSourceComboUse& locComboUseToCreate, const DSourceComboUse& locNMinus1ComboUse, const DSourceComboUse& locSourceComboDecayUse, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_Presiding);
		void Combo_Vertically_AllParticles(const DSourceComboUse& locComboUseToCreate, ComboingStage_t locComboingStage);
		void Combo_Vertically_NParticles(const DSourceComboUse& locComboUseToCreate, const DSourceComboUse& locNMinus1ComboUse, ComboingStage_t locComboingStage);

		//COMBO HORIZONTALLY METHODS
		void Combo_Horizontally_All(const DSourceComboUse& locComboUseToCreate, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_Presiding);
		void Create_Combo_OneParticle(const DSourceComboUse& locComboUseToCreate, ComboingStage_t locComboingStage);
		void Combo_Horizontally_AddCombo(const DSourceComboUse& locComboUseToCreate, const DSourceComboUse& locAllBut1ComboUse, const DSourceComboUse& locSourceComboUseToAdd, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_Presiding, bool locExpandAllBut1Flag);
		void Combo_Horizontally_AddParticle(const DSourceComboUse& locComboUseToCreate, const DSourceComboUse& locAllBut1ComboUse, Particle_t locPID, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_Presiding);

		//BUILD/RETRIEVE RESUME-AT ITERATORS
		void Build_ParticleIterators(const vector<int>& locBeamBunches, const vector<const JObject*>& locParticles);
		void Build_ComboIterators(const vector<int>& locBeamBunches, const vector<const DSourceCombo*>& locCombos, ComboingStage_t locComboingStage, signed char locVertexZBin);
		vector<const JObject*>::const_iterator Get_ResumeAtIterator_Particles(const DSourceCombo* locSourceCombo, const vector<int>& locBeamBunches) const;
		vector<const DSourceCombo*>::const_iterator Get_ResumeAtIterator_Combos(const DSourceCombo* locSourceCombo, const vector<int>& locBeamBunches, ComboingStage_t locComboingStage, signed char locVertexZBin) const;

		//GET POTENTIAL PARTICLES & COMBOS FOR COMBOING
		const vector<const DSourceCombo*>& Get_CombosForComboing(const DSourceComboUse& locComboUse, ComboingStage_t locComboingStage, const vector<int>& locBeamBunches, const DSourceCombo* locChargedCombo_WithPrevious);
		const vector<const DSourceCombo*>& Get_CombosByBeamBunch(DCombosByBeamBunch& locCombosByBunch, const vector<int>& locBeamBunches, ComboingStage_t locComboingStage, signed char locVertexZBin);

		//REGISTER VALID RF BUNCHES
		void Register_ValidRFBunches(const DSourceComboUse& locSourceComboUse, const DSourceCombo* locSourceCombo, const vector<int>& locRFBunches, ComboingStage_t locComboingStage, const DSourceCombo* locChargedCombo_WithNow);

		//PARTICLE UTILITY FUNCTIONS
		const vector<const JObject*>& Get_ParticlesForComboing(Particle_t locPID, ComboingStage_t locComboingStage, const vector<int>& locBeamBunches = {}, signed char locVertexZBin = 0);
		const vector<const JObject*>& Get_ShowersByBeamBunch(const vector<int>& locBeamBunches, DPhotonShowersByBeamBunch& locShowersByBunch);
		shared_ptr<const DKinematicData> Create_KinematicData(const DNeutralShower* locNeutralShower, const DVector3& locVertex) const;
		bool Get_IsComboingZIndependent(const JObject* locObject, Particle_t locPID) const;

		//COMBO UTILITY FUNCTIONS
		DSourceCombosByUse_Large& Get_CombosSoFar(ComboingStage_t locComboingStage, Charge_t locChargeContent_SearchForUse, const DSourceCombo* locChargedCombo = nullptr);
		DSourceCombosByBeamBunchByUse& Get_SourceCombosByBeamBunchByUse(Charge_t locChargeContent_SearchForUse, const DSourceCombo* locChargedCombo = nullptr);
		void Copy_ZIndependentMixedResults(const DSourceComboUse& locComboUseToCreate, const DSourceCombo* locChargedCombo_WithNow);
		const DSourceCombo* Get_ChargedCombo_WithNow(const DSourceCombo* locChargedCombo_Presiding) const;
		const DSourceCombo* Get_Presiding_ChargedCombo(const DSourceCombo* locChargedCombo_Presiding, const DSourceComboUse& locNextComboUse, ComboingStage_t locComboingStage, size_t locInstance) const;
		bool Get_PromoteFlag(Particle_t locDecayPID_UseToCheck, const DSourceComboInfo* locComboInfo_UseToCreate, const DSourceComboInfo* locComboInfo_UseToCheck) const;

		/************************************************************** DEFINE MEMBERS ***************************************************************/

		//CONTROL INFORMATION
		uint64_t dEventNumber = 0; //re-setup on new events
		string dShowerSelectionTag = "PreSelect";
		size_t dDebugLevel = 0;

		//EXPERIMENT INFORMATION
		DVector3 dTargetCenter;

		//COMMAND LINE CUTS
		pair<bool, size_t> dNumPlusMinusRFBunches = std::make_pair(false, 0); //by default use DReaction cut //only use this if set on command line
		unordered_map<const DReaction*, size_t> dRFBunchCutsByReaction;
		unordered_map<const DReactionVertexInfo*, size_t> dMaxRFBunchCuts;

		//VERTEX-DEPENDENT PHOTON INFORMATION
		//For every 10cm in vertex-z, calculate the photon p4 & time for placing mass & delta-t cuts
		//The z-range extends from the upstream end of the target - 5cm to the downstream end + 15cm
		//so for a 30-cm-long target, it's a range of 50cm: 5bins, evaluated at the center of each bin
		//Make sure that the center of the target is the center of a zbin!!!
		float dPhotonVertexZBinWidth;
		float dPhotonVertexZRangeLow;
		size_t dNumPhotonVertexZBins;

		//HANDLERS AND VERTEXERS
		DSourceComboVertexer* dSourceComboVertexer;
		DSourceComboP4Handler* dSourceComboP4Handler;
		DSourceComboTimeHandler* dSourceComboTimeHandler;
		DParticleComboCreator* dParticleComboCreator;

		//SOURCE COMBO INFOS: CREATED ONCE DURING DSOURCECOMBOER OBJECT CONSTRUCTION
			//with some exceptions (specific vertex-z, etc.)
		//want to make sure we only have one of each type: suggests using a set
		//however, after the first few events, almost all of these have already been created: vector has faster lookup time
		//therefore, use the set when creating the objects during construction, but then move the results into the vector and keep it sorted
		set<const DSourceComboInfo*, DCompare_SourceComboInfos> dSourceComboInfoSet;
		vector<const DSourceComboInfo*> dSourceComboInfos;
		unordered_map<const DSourceComboInfo*, Charge_t> dComboInfoChargeContent;
		unordered_set<const DSourceComboInfo*> dComboInfosWithMassiveNeutrals;
		//the rest
		unordered_map<const DReactionStepVertexInfo*, DSourceComboUse> dSourceComboUseReactionMap; //primary combo info (nullptr if none)
		//combo use -> step
		map<pair<const DReactionStepVertexInfo*, DSourceComboUse>, size_t> dSourceComboInfoStepMap; //size_t: step index
		//i need to go from step -> combo use
		unordered_map<const DReaction*, map<size_t, DSourceComboUse>> dSourceComboUseReactionStepMap; //primary combo info (nullptr if none)
		//with specific vertex-z's
		map<pair<const DReactionVertexInfo*, vector<signed char>>, DSourceComboUse> dSourceComboUseVertexZMap;
		map<DSourceComboUse, DSourceComboUse> dZDependentUseToIndependentMap; //from z-dependent -> z-independent

		//SKIM INFORMATION
		const DESSkimData* dESSkimData = nullptr;

		//PARTICLES
		map<Particle_t, vector<const JObject*>> dTracksByPID;
		unordered_map<signed char, DPhotonShowersByBeamBunch> dShowersByBeamBunchByZBin; //char: zbin

		//SOURCE COMBOS //vector: z-bin //if attempted and all failed, DSourceCombosByUse_Large vector will be empty
		size_t dInitialComboVectorCapacity = 100;
		DSourceCombosByUse_Large dSourceCombosByUse_Charged;
		unordered_map<const DSourceCombo*, DSourceCombosByUse_Large> dMixedCombosByUseByChargedCombo; //key: charged combo //value: contains mixed & neutral combos //neutral: key is nullptr
		//also, sort by which beam bunches they are valid for: that way when comboing, we can retrieve only the combos that can possibly match the input RF bunches
		unordered_map<const DSourceCombo*, DSourceCombosByBeamBunchByUse> dSourceCombosByBeamBunchByUse; //key: charged combo //value: contains mixed & neutral combos: key is nullptr
		map<pair<const DSourceCombo*, const DReactionStepVertexInfo*>, const DSourceCombo*> dVertexPrimaryComboMap; //first combo: reaction primary combo (can be charged or full!)

		//RESUME SEARCH ITERATORS
		//e.g. if a DSourceCombo is -> 2pi0, and we want to use it as a basis for building a combo of 3pi0s,
		//then this iterator points to the first pi0 in the DSourceCombosByUse_Large vector that we want to test
		//that way we save a lot of time, since we don't have to look for it again
		//they are useful when comboing VERTICALLY, but cannot be used when comboing HORIZONTALLY
			//e.g. when comboing a pi0 (with photons = A, D) with a single photon, the photon could be B, C, or E+: no single spot to resume at
		map<pair<const JObject*, vector<int>>, vector<const JObject*>::const_iterator> dResumeSearchAfterIterators_Particles; //vector<int>: RF bunches (empty for all)
		map<pair<const DSourceCombo*, signed char>, DComboIteratorsByBeamBunch> dResumeSearchAfterIterators_Combos; //char: zbin

		//RESUME-SEARCH-AFTER OBJECTS
		//The below resume-at vectors are only useful when comboing vertically, so PID-specific versions are not needed
		unordered_map<const DSourceCombo*, const DSourceCombo*> dResumeSearchAfterMap_Combos; //key: Combo containing N combos of the type in the value //value: the last of those N combos
		unordered_map<const DSourceCombo*, const JObject*> dResumeSearchAfterMap_Particles; //key: Combo containing N particles of the type in the value //value: the last of those N particles

		//VALID RF BUNCHES BY COMBO
		unordered_map<const DSourceCombo*, vector<int>> dValidRFBunches_ByCombo;

		//RESOURCE POOLS
		DResourcePool<DSourceCombo> dResourcePool_SourceCombo;
		DResourcePool<vector<const DSourceCombo*>> dResourcePool_SourceComboVector;
};

/*********************************************************** INLINE MEMBER FUNCTION DEFINITIONS ************************************************************/

inline const DSourceCombo* DSourceComboer::Get_ChargedCombo_WithNow(const DSourceCombo* locChargedCombo_Presiding) const
{
	if(locChargedCombo_Presiding == nullptr)
		return nullptr;

	for(const auto& locFurtherDecayPair : locChargedCombo_Presiding->Get_FurtherDecayCombos())
	{
		if(dComboInfoChargeContent.find(std::get<2>(locFurtherDecayPair.first))->second == d_Charged)
			return locFurtherDecayPair.second[0]; //guaranteed to be size = 1
	}
	return nullptr; //uh oh ...
}


inline size_t DSourceComboer::Get_PhotonVertexZBin(double locVertexZ) const
{
	//given some vertex-z, what bin am I in?
	int locPhotonVertexZBin = int((locVertexZ - dPhotonVertexZRangeLow)/dPhotonVertexZBinWidth);
	if(locPhotonVertexZBin < 0)
		return 0;
	else if(locPhotonVertexZBin >= int(dNumPhotonVertexZBins))
		return dNumPhotonVertexZBins - 1;
	return locPhotonVertexZBin;
}

inline double DSourceComboer::Get_PhotonVertexZBinCenter(signed char locVertexZBin) const
{
	return dPhotonVertexZRangeLow + (double(locVertexZBin) + 0.5)*dPhotonVertexZBinWidth;
}

inline bool DSourceComboer::Check_NumParticles(const DReaction* locReaction)
{
	//see if enough particles were detected to build this reaction
	auto locReactionPIDs = locReaction->Get_FinalPIDs(-1, false, false, d_AllCharges, true); //no missing, no decaying, include duplicates
	auto locPIDMap = DAnalysis::Convert_VectorToCountMap<Particle_t>(locReactionPIDs);
	if(dDebugLevel >= 5)
		cout << "Checking #particles" << endl;
	for(const auto& locPIDPair : locPIDMap)
	{
		auto locNumParticlesForComboing = Get_ParticlesForComboing(locPIDPair.first, d_MixedStage).size();
		if(dDebugLevel >= 5)
			cout << ParticleType(locPIDPair.first) << ": Need " << locPIDPair.second << ", Have " << locNumParticlesForComboing << endl;
		if(locNumParticlesForComboing < locPIDPair.second)
			return false;
	}
	return true;
}


inline bool DSourceComboer::Check_Skims(const DReaction* locReaction) const
{
	if(dESSkimData == nullptr)
		return true;

	string locReactionSkimString = locReaction->Get_EventStoreSkims();
	vector<string> locReactionSkimVector;
	SplitString(locReactionSkimString, locReactionSkimVector, ",");
	for(size_t loc_j = 0; loc_j < locReactionSkimVector.size(); ++loc_j)
	{
		if(!dESSkimData->Get_IsEventSkim(locReactionSkimVector[loc_j]))
			return false;
	}

	return true;
}

inline void DSourceComboer::Build_ParticleIterators(const vector<int>& locBeamBunches, const vector<const JObject*>& locParticles)
{
	for(vector<const JObject*>::const_iterator locIterator = locParticles.begin(); locIterator != locParticles.end(); ++locIterator)
		dResumeSearchAfterIterators_Particles.emplace(std::make_pair(*locIterator, locBeamBunches), locIterator);
}

inline void DSourceComboer::Build_ComboIterators(const vector<int>& locBeamBunches, const vector<const DSourceCombo*>& locCombos, ComboingStage_t locComboingStage, signed char locVertexZBin)
{
	for(vector<const DSourceCombo*>::const_iterator locIterator = locCombos.begin(); locIterator != locCombos.end(); ++locIterator)
		dResumeSearchAfterIterators_Combos[std::make_pair(*locIterator, locVertexZBin)].emplace(locBeamBunches, locIterator);
}

inline vector<const JObject*>::const_iterator DSourceComboer::Get_ResumeAtIterator_Particles(const DSourceCombo* locSourceCombo, const vector<int>& locBeamBunches) const
{
	const auto& locPreviousObject = dResumeSearchAfterMap_Particles.find(locSourceCombo)->second;
	return std::next(dResumeSearchAfterIterators_Particles.find(std::make_pair(locPreviousObject, locBeamBunches))->second);
}

inline vector<const DSourceCombo*>::const_iterator DSourceComboer::Get_ResumeAtIterator_Combos(const DSourceCombo* locSourceCombo, const vector<int>& locBeamBunches, ComboingStage_t locComboingStage, signed char locVertexZBin) const
{
	const auto& locPreviousCombo = dResumeSearchAfterMap_Combos.find(locSourceCombo)->second;
	return std::next(dResumeSearchAfterIterators_Combos.find(std::make_pair(locPreviousCombo, locVertexZBin))->second.find(locBeamBunches)->second);
}

inline bool DSourceComboer::Get_IsComboingZIndependent(const JObject* locObject, Particle_t locPID) const
{
	//make this virtual!
	if(ParticleCharge(locPID) != 0)
		return true;

	//actually, everything is z-dependent for massive neutrals.
	//however the momentum is SO z-dependent, that we can't cut on it until the end when we have the final vertex, AFTER comboing
	//a mere z-bin is not enough.
	//So, as far as COMBOING is concerned, massive neutrals are Z-INDEPENDENT
	if(ParticleMass(locPID) > 0.0)
		return true;

	auto locNeutralShower = static_cast<const DNeutralShower*>(locObject);
	return (locNeutralShower->dDetectorSystem == SYS_FCAL);
}

inline DSourceCombosByUse_Large& DSourceComboer::Get_CombosSoFar(ComboingStage_t locComboingStage, Charge_t locChargeContent_SearchForUse, const DSourceCombo* locChargedCombo)
{
	//THE INPUT locChargedCombo MUST BE:
	//If reading from: Whatever charged combo you PREVIOUSLY comboed horizontally with to make the combos you're trying to get
	//If saving to (you are making a mixed): Whatever charged combo you are about to combo horizontally with to make this new, mixed combo

	//NOTE: If on mixed stage, it is NOT valid to get fully-charged combos from here! In fact, what you want is probably the input combo!
	if(locComboingStage == d_ChargedStage)
		return dSourceCombosByUse_Charged;
	else if(locChargeContent_SearchForUse == d_Neutral)
		return dMixedCombosByUseByChargedCombo[nullptr]; //if fully neutral, then the charged combo doesn't matter: only matters for mixing charges
	return dMixedCombosByUseByChargedCombo[locChargedCombo];
}

inline DSourceCombosByBeamBunchByUse& DSourceComboer::Get_SourceCombosByBeamBunchByUse(Charge_t locChargeContent_SearchForUse, const DSourceCombo* locChargedCombo)
{
//shouldn't ever be called if charged
	//THE INPUT locChargedCombo MUST BE:
	//If reading from: Whatever charged combo you PREVIOUSLY comboed horizontally with to make the combos you're trying to get
	//If saving to (you are making a mixed): Whatever charged combo you just comboed horizontally with to make this new, mixed combo
	if(locChargeContent_SearchForUse == d_Neutral)
		return dSourceCombosByBeamBunchByUse[nullptr];
	return dSourceCombosByBeamBunchByUse[locChargedCombo];
}

inline void DSourceComboer::Recycle_ComboResources(DSourceCombosByUse_Large& locCombosByUse)
{
	for(auto& locCombosByUsePair : locCombosByUse)
	{
		//the combos after mass cuts are IDENTICAL to those before the mass cuts: the pointers are just copied
		//don't recycle them twice!: Only recycle combos stored for a use with an Unknown decay PID
		if(std::get<0>(locCombosByUsePair.first) != Unknown)
			continue; //don't recycle! would recycle the same pointers twice!

		//recycle the combos
		auto locVectorPointer = locCombosByUsePair.second;
		dResourcePool_SourceCombo.Recycle(*locVectorPointer);

		//the above MOVED the resources out of the vector, and cleared the vector: it now has a capacity of zero
		dResourcePool_SourceComboVector.Recycle(locVectorPointer); //recycle the combo vector
	}
}

inline DSourceComboer::~DSourceComboer(void)
{
	//no need for a resource pool for these objects, as they will exist for the length of the program
	for(auto locComboInfo : dSourceComboInfos)
		delete locComboInfo;
}


/*********************************************************** INLINE NAMESPACE-SCOPE FUNCTION DEFINITIONS ************************************************************/

} //end DAnalysis namespace

#endif // DSourceComboer_h