#ifndef DEF_NN_PCONFIG
#define DEF_NN_PCONFIG

#include <vector>
#include "TString.h"

#include "ConfigFile.h"

class PConfig{
	public:
	
	PConfig(TString configFile);

	TString GetPath(unsigned int i) const;
	TString GetName(unsigned int i) const;
	int GetType(unsigned int i) const;
	Color_t GetColor(unsigned int i) const;
	double GetXSection(unsigned int i) const;
	long GetTotEvents(unsigned int i) const;
	TString GetTreeName(unsigned int i) const;
	
	unsigned int GetNData(void) const;
	TString GetAnaName(void) const;
	TString GetOutputDir(void) const;
	TString GetOutputName(void) const;
	TString GetTopology(void) const;
	TString GetMvaMethod(void) const;
	unsigned int GetIterations(void) const;
	unsigned int GetTrainEntries(void) const;
	double GetWorkingPoint(void) const;
	double GetLumi(void) const;
	TString GetGenWeight(void) const;
	unsigned int GetHistBins(void) const;
	unsigned int GetPlotBins(void) const;
	TString GetWriteOptions(void) const;
	TString GetOutputTasks(void) const;
	TString GetSplitName(void) const;
	TString GetLogName(void) const;
	TString GetWeight(unsigned int i) const;
	unsigned int GetNWeights(void) const;

	private:

	std::vector<TString> paths;
	std::vector<TString> names;
	std::vector<int> types;
	std::vector<Color_t> colors;
	std::vector<double> xSections;
	std::vector<long> totEvents;
	std::vector<TString> treeNames;
	unsigned int nData, nWeights;

	TString anaName, outputDir, outputName, topology, mvaMethod;
	unsigned int iterations, trainEntries;
	double workingPoint, lumi;
	unsigned int histBins, plotBins;
	TString writeOptions, outputTasks, splitName, logName, genWeight;
	std::vector<TString> weights;

	Color_t TranslateColor(TString color) const;
};

#endif