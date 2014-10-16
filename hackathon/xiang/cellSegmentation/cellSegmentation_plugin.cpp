/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 : by Xiang Li (lindbergh.li@gmail.com);
 */
 
#pragma region "headers and constant defination"
#include "v3d_message.h"
#include "cellSegmentation_plugin.h"
#include <vector>
#include <math.h>
#include "string"
#include "sstream"
#include <time.h>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include <fstream>
#include <stdlib.h>
#include <ctime>
#include <basic_landmark.h>
using namespace std;

const int const_int_histogramLength = 256;
const int const_int_histogramInterval = 3;
const int const_int_floatCriteria = 65534;
const int const_int_longCriteria = 254;
const double const_double_maxPixelIntensity = 255;
const int const_int_defaultMinimumVolume = 40;
const int const_int_count_neighbors = 26;
const int const_int_maximumGrowingLength = 10000;
const int const_int_minimumIntensityConsideredAsNonZero = 2;
const double const_double_valueChangeThreshold = 0.4;
const double const_double_preThresholdLoosenBy = 0.45;
const double const_double_fragmentRatioToMinExemplerRegionLoosenBy = 0.05;
const double const_double_regioinRatioToMinExemplerRegionLoosenBy = 1;
const double const_double_fragmentRatioToPageSizeWarning = 0.01;
const double const_double_regionFittingTolerateLevel = 0.95;

#define INF 1E9
#define NINF -1E9
#define PI 3.14159265

#define UBYTE unsigned char
#define BYTE signed char
#pragma endregion

#pragma region "UI-related functions and function defination"
Q_EXPORT_PLUGIN2(cellSegmentation, cellSegmentation);

bool func_interface_Segmentation(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent);
bool subFunc_NeuronTree2LandmarkList(const NeuronTree & NeuronTree_input, LandmarkList & LandmarkList_output);
void func_Image1DVisualization(unsigned char* Image1D_input, V3DLONG int_xDim, V3DLONG int_yDim, V3DLONG int_zDim, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback);
void func_regionListVisualization(vector<V3DLONG> vct_regionInput, V3DLONG int_xDim, V3DLONG int_yDim, V3DLONG int_zDim, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName);
void func_regionVisualization(vector<V3DLONG> vct_regionInput, V3DLONG int_xDim, V3DLONG int_yDim, V3DLONG int_zDim, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName);
unsigned char* subFunc_Image1DTypeConversion(unsigned short *Image1D_input, V3DLONG count_total);
unsigned char* subFunc_Image1DTypeConversion(float *Image1D_input, V3DLONG count_total);

QStringList cellSegmentation::menulist() const
{
    return QStringList()
        <<tr("Cell segmentation")
        <<tr("About");
}

QStringList cellSegmentation::funclist() const
{
	return QStringList()
		<<tr("cellsegmentation")
		<<tr("help");
}

void cellSegmentation::domenu(const QString &menu_name, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
{
    if (menu_name == tr("Cell segmentation"))
	{
        func_interface_Segmentation(V3DPluginCallback2_currentCallback,QWidget_parent);
    }
	else
	{
        v3d_msg(tr("Segmenting neurons;"
            "by Xiang Li (lindbergh.li@gmail.com);"
			));
	}
}

bool cellSegmentation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & V3DPluginCallback2_currentCallback,  QWidget * QWidget_parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("cellsegmentation"))
	{
		
	}
	else if (func_name == tr("help"))
	{
		
	}
	else
	{
		return false;
	}
	return true;
}
#pragma endregion

#pragma region "class: class_segmentationMain"
class class_segmentationMain
{
	public:
		double double_threshold;
		unsigned char* Image1D_original;
		float* Image1D_resultFloat;
		//unsigned short *Image1D_resultShort;
		//unsigned char *Image1D_resultChar;
		V3DLONG int_channel;
		LandmarkList LandmarkList_exempler;
		LandmarkList LandmarkList_segmentation;
		vector<V3DLONG> vct_invalidExempler;
		ROIList ROIList_main;
		V3DLONG int_fragmentSizeThreshold;
		V3DLONG int_xDim;
		V3DLONG int_yDim;
		V3DLONG int_zDim;
		int int_resultType;
		V3DLONG count_exempler;
		V3DLONG int_totalPageSize;
		V3DLONG int_offsetByChannel;
		V3DLONG int_offsetByZ;
		V3DLONG int_offsetByY;
		unsigned char *Image1D_main;
		float *Image1D_exemplerRegion;
		unsigned char *Image1D_mask;
		vector<V3DLONG> histo_exempler;
		vector<V3DLONG> histo_main;
		vector<V3DLONG> vct_exemplerIndex;
		vector<V3DLONG> vct_exemplerRegionsVolume;
		vector<V3DLONG> vct_exemplerRegionMassCenterIndex;
		vector<vector<V3DLONG>> vctList_exemplerRegions;
		vector<vector<V3DLONG>> vctList_exemplerRegionsCentralized;
		vector<V3DLONG*> vctList_grownRegions;
		vector<V3DLONG> vct_grownRegionsVolumes;
		V3DLONG count_grownRegions;
		V3DLONG vct_neighbors[const_int_count_neighbors];
		V3DLONG count_exemplerRegionVoxels;
		V3DLONG int_globalThresholdValue;
		V3DLONG int_minExemplerRegionVolume;
		vector<vector<V3DLONG>> vctList_segmentation;
		V3DLONG count_segmentation;
		vector<V3DLONG> vct_segmentationVolume;
		
		class_segmentationMain(double double_thresholdInput, unsigned char* Image1D_input, V3DLONG int_xDimInput, V3DLONG int_yDimInput, V3DLONG int_zDimInput , int int_channelInput, LandmarkList LandmarkList_input)
		{
			this->Image1D_original = Image1D_input;
			this->int_xDim = int_xDimInput;
			this->int_yDim = int_yDimInput;
			this->int_zDim = int_zDimInput;
			this->int_channel = int_channelInput;
			this->LandmarkList_exempler = LandmarkList_input;
			this->int_totalPageSize = int_xDim*int_yDim*int_zDim;
			this->int_offsetByChannel = (int_channel-1)*int_totalPageSize;
			this->int_offsetByZ = int_xDim*int_yDim;
			this->int_offsetByY = int_xDim;
			this->int_fragmentSizeThreshold = const_int_defaultMinimumVolume;
			this->Image1D_main = new unsigned char [this->int_totalPageSize];
			this->Image1D_mask = new unsigned char [this->int_totalPageSize];
			this->Image1D_exemplerRegion = new float [this->int_totalPageSize];
			for (V3DLONG i=0;i<int_totalPageSize;i++)
			{	
				Image1D_main[i] = Image1D_original[i+int_offsetByChannel];
				Image1D_mask[i] = 0; //available;
				Image1D_exemplerRegion[i] = 0;
			}
			this->getHistogramOfMain();
			int_globalThresholdValue = estimateThresholdYen(histo_main);
			count_exempler = LandmarkList_exempler.count();
			vct_exemplerIndex = landMarkList2IndexList(LandmarkList_exempler);
			V3DLONG idx_neighbor = 0;
			for (V3DLONG z=-1;z<=1;z++)
			{
				for (V3DLONG y=-1;y<=1;y++)
				{
					for (V3DLONG x=-1;x<=1;x++)
					{
						if (x==0&&y==0&&z==0)
						{
							//that's itself;
						}
						else
						{
							vct_neighbors[idx_neighbor] = z*int_offsetByZ+y*int_offsetByY+x;
							idx_neighbor = idx_neighbor + 1;
						}
					}
				}
			}
			regonGrowOnExempler();
			if (double_thresholdInput < 0)
			{
				getHistogramForExemplerRegion();
				this->double_threshold = estimateThresholdYen(histo_exempler);
				if (this->double_threshold < this->int_globalThresholdValue)
				{
					this->double_threshold = this->int_globalThresholdValue;
				}
			}
			else
			{
				this->double_threshold = double_thresholdInput;
			}
		}

		void regionFitting()
		{
			//ofstream ofstream_log;
			//ofstream_log.open ("log_regionFitting.txt");
			V3DLONG idx_tmp;
			V3DLONG count_hit = 0;
			V3DLONG idx_hitExempler = 0;
			V3DLONG int_label;
			this->vctList_segmentation.clear();
			vector<V3DLONG> vct_tempResult (0, 0);
			this->vct_segmentationVolume.clear();
			this->count_segmentation = 0;
			for (int idx_grownRegion=0;idx_grownRegion<this->count_grownRegions;idx_grownRegion++)
			{
				//ofstream_log<<"idx_grownRegion: "<<idx_grownRegion<<endl;
				int_label = idx_grownRegion+1;
				if (this->vct_grownRegionsVolumes[idx_grownRegion]<(this->int_minExemplerRegionVolume*const_double_regioinRatioToMinExemplerRegionLoosenBy))
				{
					//already good enough, scoop it out, store as new segment;
					vct_tempResult.clear();
					for (int i=0;i<vct_grownRegionsVolumes[idx_grownRegion];i++)
					{
						Image1D_mask[this->vctList_grownRegions[idx_grownRegion][i]]=2; //scooped;
						vct_tempResult.push_back(this->vctList_grownRegions[idx_grownRegion][i]);
					}
					this->vctList_segmentation.push_back(vct_tempResult);
					count_segmentation++;
					vct_segmentationVolume.push_back(vct_grownRegionsVolumes[idx_grownRegion]);
				}
				//ofstream_log<<"int_label: "<<int_label<<endl;
				for (int idx_grownRegionVoxel=0;idx_grownRegionVoxel<this->vct_grownRegionsVolumes[idx_grownRegion];idx_grownRegionVoxel++)
				{
					//ofstream_log<<"idx_grownRegionVoxel: "<<idx_grownRegionVoxel<<endl;
					idx_tmp = this->vctList_grownRegions[idx_grownRegion][idx_grownRegionVoxel];
					//ofstream_log<<"idx_tmp: "<<idx_tmp<<endl;
					if ((idx_tmp<0) || (idx_tmp>=this->int_totalPageSize))
					{
						v3d_msg("Waring: error occured during examination of grown regions. Please terminate the program immediately!!!");
						return;
					}
					//ofstream_log<<"hit if (this->Image1D_mask[idx_tmp] < 1)"<<endl;
					if (this->Image1D_mask[idx_tmp] < 1) //available;
					{
						//ofstream_log<<"if (this->Image1D_mask[idx_tmp] < 1): true"<<endl;
						for (V3DLONG idx_region=0;idx_region<this->count_exempler;idx_region++)
						{
							//ofstream_log<<"idx_region: "<<idx_region<<endl;
							count_hit = 0;
							vct_tempResult.clear();
							vct_tempResult.push_back(idx_tmp);
							for (V3DLONG idx_voxel=0;idx_voxel<this->vct_exemplerRegionsVolume[idx_region];idx_voxel++)
							{
								//ofstream_log<<"idx_voxel: "<<idx_voxel<<endl;
								idx_hitExempler = idx_tmp + vctList_exemplerRegionsCentralized[idx_region][idx_voxel];
								//ofstream_log<<"idx_hitExempler: "<<idx_hitExempler<<endl;
								//ofstream_log<<"hit if ((idx_hitExempler<0)||(idx_hitExempler>=this->int_totalPageSize))"<<endl;
								if ((idx_hitExempler<0)||(idx_hitExempler>=this->int_totalPageSize))
								{
									//ofstream_log<<"if ((idx_hitExempler<0)||(idx_hitExempler>=this->int_totalPageSize)): false"<<endl;
									//it's out of bounds;
								}
								else
								{
									//ofstream_log<<"if ((idx_hitExempler<0)||(idx_hitExempler>=this->int_totalPageSize)): true"<<endl;
									//ofstream_log<<"hit if ((this->Image1D_mask[idx_hitExempler]<1)&&(this->Image1D_resultFloat[idx_hitExempler]<(int_label+1))&&(this->Image1D_resultFloat[idx_hitExempler]>(int_label-1)))"<<endl;
									if ((this->Image1D_mask[idx_hitExempler]<1)&&(this->Image1D_resultFloat[idx_hitExempler]<(int_label+1))&&(this->Image1D_resultFloat[idx_hitExempler]>(int_label-1))) //available and belonging to the same segments by regionGrowing;
									{
										//ofstream_log<<"if ((this->Image1D_mask[idx_hitExempler]<1)&&(this->Image1D_resultFloat[idx_hitExempler]<(int_label+1))&&(this->Image1D_resultFloat[idx_hitExempler]>(int_label-1))): true"<<endl;
										count_hit = count_hit + 1;
										vct_tempResult.push_back(idx_hitExempler);
									}
								}
							}
							//ofstream_log<<"count_hit: "<<count_hit<<endl;
							if (count_hit >= (this->vct_exemplerRegionsVolume[idx_region]*const_double_regionFittingTolerateLevel))
							{
								//Scoop it out, store as new segment;
								this->vctList_segmentation.push_back(vct_tempResult);
								for (int i=0;i<count_hit;i++)
								{
									Image1D_mask[vct_tempResult[i]]=2; //scooped;
								}
								count_segmentation++;
								vct_segmentationVolume.push_back(count_hit);
							}
						}
					}
				}
			}

			for (int i=0;i<this->int_totalPageSize;i++)
			{
				this->Image1D_resultFloat[i] = 0;
			}
			for (int i=0;i<this->count_segmentation;i++)
			{
				for (int j=0;j<this->vct_segmentationVolume[i];j++)
				{
					this->Image1D_resultFloat[vctList_segmentation[i][j]] = i+1;
				}
			}
			v3d_msg(QString("segmentation complete, total regions: %1").arg(this->count_segmentation));
			//ofstream_log.close();
		}

		void analyzeExemplerRegion()
		{
			vctList_exemplerRegionsCentralized = vctList_exemplerRegions;
			vct_exemplerRegionMassCenterIndex.resize(vctList_exemplerRegions.size());
			V3DLONG int_indexTmp;
			vector<V3DLONG> vct_xyz;
			double double_valueTmp;
			double double_weightedX = 0;
			double double_weightedY = 0;
			double double_weightedZ = 0;
			double double_massSum = 0;
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			V3DLONG xMax;
			V3DLONG yMax;
			V3DLONG zMax;
			V3DLONG xMin;
			V3DLONG yMin;
			V3DLONG zMin;
			int_minExemplerRegionVolume = INF;
			V3DLONG int_volumeTmp;
			for (V3DLONG idx_region=0;idx_region<vctList_exemplerRegions.size();idx_region++)
			{
				double_weightedX = 0;
				double_weightedY = 0;
				double_weightedZ = 0;
				double_massSum = 0;
				xMax = -INF;
				yMax = -INF;
				zMax = -INF;
				xMin = INF;
				yMin = INF;
				zMin = INF;
				for (V3DLONG idx_voxel=0;idx_voxel<vctList_exemplerRegions[idx_region].size();idx_voxel++)
				{
					int_indexTmp = vctList_exemplerRegions[idx_region][idx_voxel];
					double_valueTmp = (double) this->Image1D_main[int_indexTmp]; //changed to main as we want the real value rather than the binarized one;
					vct_xyz = convertCoordiante1DTo3D(int_indexTmp);
					x = vct_xyz[0];
					y = vct_xyz[1];
					z = vct_xyz[2];
					double_weightedX += x*double_valueTmp;
					double_weightedY += y*double_valueTmp;
					double_weightedZ += z*double_valueTmp;
					double_massSum += double_valueTmp;
				}
				double_weightedX = double_weightedX/double_massSum;
				double_weightedY = double_weightedY/double_massSum;
				double_weightedZ = double_weightedZ/double_massSum;
				vct_exemplerRegionMassCenterIndex.push_back(convertCoordinateToIndex((V3DLONG)double_weightedX, (V3DLONG)double_weightedY, (V3DLONG)double_weightedZ));
				for (V3DLONG idx_voxel=0;idx_voxel<vctList_exemplerRegionsCentralized[idx_region].size();idx_voxel++)
				{
					int_indexTmp = vctList_exemplerRegionsCentralized[idx_region][idx_voxel];
					vct_xyz = convertCoordiante1DTo3D(int_indexTmp);
					x = vct_xyz[0] - double_weightedX;
					y = vct_xyz[1] - double_weightedY;
					z = vct_xyz[2] - double_weightedZ;
					if (x>xMax)
					{
						xMax=x;
					}
					if (y>yMax)
					{
						yMax=y;
					}
					if (z>zMax)
					{
						zMax=z;
					}
					if (x<xMin)
					{
						xMin=x;
					}
					if (y<yMin)
					{
						yMin=y;
					}
					if (z<zMin)
					{
						zMin=z;
					}
					vctList_exemplerRegionsCentralized[idx_region][idx_voxel] = convertCoordinateToIndex(x, y, z);
				}
				int_volumeTmp = (xMax-xMin)*(yMax-yMin)*(zMax-zMin);
				if (int_volumeTmp<int_minExemplerRegionVolume)
				{
					int_minExemplerRegionVolume = int_volumeTmp;
				}
			}
			if (int_minExemplerRegionVolume>=(this->int_totalPageSize*const_double_fragmentRatioToPageSizeWarning))
			{
				v3d_msg("Warning: abnormally large exempler region size detected, results might be inaccurate. Please re-run the program!!!");
			}
			this->int_fragmentSizeThreshold = int_minExemplerRegionVolume*const_double_fragmentRatioToMinExemplerRegionLoosenBy;
		}

		void regonGrowOnExempler()
		{
			vector<V3DLONG> vct_temp;
			V3DLONG idx_exempler;
			count_exemplerRegionVoxels = 0;
			for (V3DLONG i=0;i<count_exempler;i++)
			{
				idx_exempler = vct_exemplerIndex[i];
				vct_temp = regionGrowOnSeed(idx_exempler);
				if (!vct_temp.empty())
				{
					this->vctList_exemplerRegions.push_back(vct_temp);
				}
				else
				{
					vct_invalidExempler.push_back(i);
				}
			}
			for (V3DLONG idx_region=0;idx_region<vctList_exemplerRegions.size();idx_region++)
			{
				for (V3DLONG idx_voxel=0;idx_voxel<vctList_exemplerRegions[idx_region].size();idx_voxel++)
				{
					this->Image1D_exemplerRegion[vctList_exemplerRegions[idx_region][idx_voxel]] = this->Image1D_main[vctList_exemplerRegions[idx_region][idx_voxel]];
					count_exemplerRegionVoxels = count_exemplerRegionVoxels + 1;
				}
			}
		}

		vector<V3DLONG> regionGrowOnSeed(V3DLONG idx_seed) //assuming [0-255] integer Image1D_main;
		{
			vector<V3DLONG> vct_result;
			vector<V3DLONG> vct_temp;
			V3DLONG idx_current;
			V3DLONG idx_neighbor;
			double double_neightborValue;
			double double_currentValue;
			double double_meanValue = 0;
			double double_sumValue = 0;
			double double_seedValue = Image1D_main[idx_seed];
			if (double_seedValue>const_int_minimumIntensityConsideredAsNonZero)
			{
				vct_temp.push_back(idx_seed);
				vct_result.push_back(idx_seed);
				double_sumValue = double_seedValue;
				double_meanValue = double_seedValue;
				for (V3DLONG i=0;i<const_int_maximumGrowingLength;i++)
				{
					if (vct_temp.empty())
					{
						return vct_result;
					}
					idx_current = vct_temp.back();
					double_currentValue = Image1D_main[idx_current];
					vct_temp.pop_back();
					for (V3DLONG j=0;j<const_int_count_neighbors;j++)
					{
						idx_neighbor = idx_current+vct_neighbors[j];
						if (idx_neighbor>0 && idx_neighbor<int_totalPageSize)
						{
							if (Image1D_mask[idx_neighbor]==0) //available only;
							{
								double_neightborValue = Image1D_main[idx_neighbor];
								if ((double_neightborValue > int_globalThresholdValue*const_double_preThresholdLoosenBy) && ((abs(double_neightborValue-double_currentValue))<double_meanValue*const_double_valueChangeThreshold))
								{
									Image1D_mask[idx_neighbor] = 2; //scooped;
									vct_temp.push_back(idx_neighbor);
									vct_result.push_back(idx_neighbor);
									double_sumValue = double_sumValue + double_neightborValue;
									double_meanValue = double_sumValue/vct_result.size();
								}
							}
						}
					}
				}
			}
			return vct_result;
		}

		
		vector<V3DLONG> landMarkList2IndexList(LandmarkList LandmarkList_Input)
		{
			vector<V3DLONG> vct_result;
			for (V3DLONG idx_input=0;idx_input<LandmarkList_Input.count();idx_input++)
			{
				vct_result.push_back(get1DIndexAtLandmark(LandmarkList_Input.at(idx_input)));
			}
			return vct_result;
		}

		V3DLONG get1DIndexAtLandmark(LocationSimple Landmark_Input)
		{
			float x=0;
			float y=0;
			float z=0;
			Landmark_Input.getCoord(x, y, z);
			return (convertCoordinateToIndex(x-1, y-1, z-1));
		}
	
		vector<V3DLONG> convertCoordiante1DTo3D(V3DLONG idx)
		{
			vector<V3DLONG> vct_coordinate3D (3, -1);
			if (idx > -1)
			{
				vct_coordinate3D[2] = floor((double)idx/(double)int_offsetByZ);
				vct_coordinate3D[1] = floor((double)(idx-vct_coordinate3D[2]*int_offsetByZ)/(double)int_offsetByY);
				vct_coordinate3D[0] = idx- vct_coordinate3D[2]*int_offsetByZ-vct_coordinate3D[1]*int_offsetByY;
			}
			return vct_coordinate3D;
		}

		V3DLONG convertCoordinateToIndex(V3DLONG x, V3DLONG y, V3DLONG z)
		{
			return z*int_offsetByZ+y*int_offsetByY+x;
		}

		double getValueAtCoordiante(V3DLONG x, V3DLONG y, V3DLONG z)
		{
			return (Image1D_main[convertCoordinateToIndex(x, y, z)]);
		}

		double getMeanFromIndexVector(vector<V3DLONG> vct_index)
		{
			double double_mean = 0;
			double double_valueTmp = 0;
			if (vct_index.empty())
			{
				return 0;
			}
			for (V3DLONG i=0;i<vct_index.size();i++)
			{
				double_mean = double_mean + Image1D_main[vct_index[i]];
			}
			return double_mean/vct_index.size();
		}

		vector<V3DLONG> getHistogramFromIndexVector(vector<V3DLONG> vct_index)
		{
			vector<V3DLONG> vct_result (const_int_histogramLength, 0);
			V3DLONG int_valueTmp;
			for (V3DLONG i=0;i<vct_index.size();i++)
			{
				int_valueTmp = (V3DLONG)Image1D_main[vct_index[i]];
				if (int_valueTmp>0 && int_valueTmp<const_int_histogramLength)
				{
					vct_result[int_valueTmp] = vct_result[int_valueTmp]+1;
				}
			}
			return vct_result;
		}

		void getHistogramOfMain()
		{
			this->histo_main.resize(const_int_histogramLength);
			V3DLONG int_valueTmp;
			for (V3DLONG i=0;i<const_int_histogramLength;i++)
			{
				this->histo_main[i]=0;
			}
			for (V3DLONG i=0;i<this->int_totalPageSize;i++)
			{
				int_valueTmp = Image1D_main[i];
				if (int_valueTmp>0 && int_valueTmp<const_int_histogramLength)
				{
					this->histo_main[int_valueTmp] = this->histo_main[int_valueTmp]+1;
				}
			}
		}

		void getHistogramForExemplerRegion()
		{

			histo_exempler.resize(const_int_histogramLength);
			V3DLONG int_valueTmp;
			for (V3DLONG i=0;i<const_int_histogramLength;i++)
			{
				histo_exempler[i]=0;
			}

			for (V3DLONG i=0;i<this->int_totalPageSize;i++)
			{
				int_valueTmp = (V3DLONG)Image1D_exemplerRegion[i];
				if (int_valueTmp>0 && int_valueTmp<const_int_histogramLength)
				{
					histo_exempler[int_valueTmp] = histo_exempler[int_valueTmp] + 1;
				}
			}
		}

		V3DLONG threshold2BinaryForMain()
		{
			V3DLONG count_totalWhite = 0;
			for(V3DLONG i=0; i<this->int_totalPageSize; i++)
			{	
				if ((double)Image1D_main[i]>this->double_threshold)
				{
					Image1D_main[i] = 255;
					count_totalWhite = count_totalWhite + 1;
					Image1D_mask[i] = 0; //available;
				}
				else
				{
					Image1D_main[i] = 0;
					Image1D_mask[i] = 1; //invalid;
				}
			}
			return count_totalWhite;
		}

		V3DLONG threshold2BinaryForExemplerRegion()
		{
			double double_valueTmp;
			V3DLONG idx_tmp;
			V3DLONG count_totalWhite = 0;
			count_exemplerRegionVoxels = 0;
			vector<vector<V3DLONG>> vctList_voxelRemove;
			vector<V3DLONG> vctList_regionRemove;
			vector<V3DLONG> vct_empty (0,0);
			V3DLONG count_exemplerRegionVolume;
			this->vct_exemplerRegionsVolume.clear();
			for (V3DLONG idx_region=0;idx_region<vctList_exemplerRegions.size();idx_region++)
			{
				vctList_voxelRemove.push_back(vct_empty);
				count_exemplerRegionVolume = vctList_exemplerRegions[idx_region].size();
				for (V3DLONG idx_voxel=0;idx_voxel<count_exemplerRegionVolume;idx_voxel++)
				{
					this->vct_exemplerRegionsVolume.push_back(count_exemplerRegionVolume);
					idx_tmp = vctList_exemplerRegions[idx_region][idx_voxel];
					if (idx_tmp < 0)
					{
						//It has been marked as "removed";
					}
					else
					{	
						double_valueTmp = this->Image1D_exemplerRegion[idx_tmp];
						if (double_valueTmp > this->double_threshold)
						{
							Image1D_exemplerRegion[idx_tmp] = 255;
							Image1D_mask[idx_tmp] = 2; //scooped;
							count_totalWhite = count_totalWhite + 1;
							count_exemplerRegionVoxels = count_exemplerRegionVoxels + 1;
						}
						else
						{
							vctList_exemplerRegions[idx_region][idx_voxel] = -1;
							vctList_voxelRemove[idx_region].push_back(idx_voxel);
							Image1D_exemplerRegion[idx_tmp] = 0;
							Image1D_mask[idx_tmp] = 1; //invalid;
						}
					}
				}
			}
			for (V3DLONG idx_region=0;idx_region<vctList_voxelRemove.size();idx_region++)
			{
				while (!vctList_voxelRemove[idx_region].empty())
				{
					vctList_exemplerRegions[idx_region].erase(vctList_exemplerRegions[idx_region].begin() + vctList_voxelRemove[idx_region].back());
					vctList_voxelRemove[idx_region].pop_back();
				}
				if (vctList_exemplerRegions[idx_region].empty())
				{
					vctList_regionRemove.push_back(idx_region);
				}
			}
			while (!vctList_regionRemove.empty())
			{
				vctList_exemplerRegions.erase(vctList_exemplerRegions.begin() + vctList_regionRemove.back());
				vctList_regionRemove.pop_back();
			}

			this->count_exempler = vctList_exemplerRegions.size();
			this->vct_exemplerRegionsVolume.clear();
			for (V3DLONG idx_region=0;idx_region<this->count_exempler;idx_region++)
			{
				this->vct_exemplerRegionsVolume.push_back(vctList_exemplerRegions[idx_region].size());
			}

			return count_totalWhite;
		}
		
		#pragma region "methods to estimate threshold"
		V3DLONG estimateThresholdYen(vector<V3DLONG> histo_input)
		{
			// Implements Yen's thresholding method;
			// 1) Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378;
			// 2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding Techniques and Quantitative Performance Evaluation" Journal of Electronic Imaging, 13(1): 146-165;
			
			int threshold;
			int ih, it;
			double crit;
			double max_crit;
			double* norm_histo = new double[const_int_histogramLength];
			double* P1 = new double[const_int_histogramLength];
			double* P1_sq = new double[const_int_histogramLength]; 
			double* P2_sq = new double[const_int_histogramLength]; 
			int total =0;
			for (ih = 0; ih < 256; ih++ ) 
				total+=histo_input[ih];
			for (ih = 0; ih < 256; ih++ )
				norm_histo[ih] = (double)histo_input[ih]/total;
			P1[0]=norm_histo[0];
			for (ih = 1; ih < 256; ih++ )
				P1[ih]= P1[ih-1] + norm_histo[ih];
			P1_sq[0]=norm_histo[0]*norm_histo[0];
			for (ih = 1; ih < 256; ih++ )
				P1_sq[ih]= P1_sq[ih-1] + norm_histo[ih] * norm_histo[ih];
			P2_sq[255] = 0.0;
			for ( ih = 254; ih >= 0; ih-- )
				P2_sq[ih] = P2_sq[ih + 1] + norm_histo[ih + 1] * norm_histo[ih + 1];
			threshold = -1;
			max_crit = NINF;
			for ( it = 0; it < 256; it++ ) {
				crit = -1.0 * (( P1_sq[it] * P2_sq[it] )> 0.0? log( P1_sq[it] * P2_sq[it]):0.0) +  2 * ( ( P1[it] * ( 1.0 - P1[it] ) )>0.0? log(  P1[it] * ( 1.0 - P1[it] ) ): 0.0);
				if ( crit > max_crit ) {
					max_crit = crit;
					threshold = it;
				}
			}
			return threshold;
		}
		#pragma endregion

		bool regionGrow()
		{	
			RgnGrow3dClass * pRgnGrow = new RgnGrow3dClass;
			if (!pRgnGrow)
			{
				v3d_msg("Fail to allocate memory for RgnGrow3dClass(), program canceled!!!");
				return false;
			}
			pRgnGrow->ImgDep = this->int_zDim;
			pRgnGrow->ImgHei = this->int_yDim;
			pRgnGrow->ImgWid = this->int_xDim;
			if (newIntImage3dPairMatlabProtocol(pRgnGrow->quantImg3d,pRgnGrow->quantImg1d,pRgnGrow->ImgDep,pRgnGrow->ImgHei,pRgnGrow->ImgWid)==0) return false;
			int nstate;
			UBYTE minlevel,maxlevel;
			copyvecdata((unsigned char *)this->Image1D_main,this->int_totalPageSize,pRgnGrow->quantImg1d,nstate,minlevel,maxlevel);
			minlevel = minlevel+1;
			if (minlevel>maxlevel)
				minlevel = maxlevel;
			if (newIntImage3dPairMatlabProtocol(pRgnGrow->PHCLABELSTACK3d,pRgnGrow->PHCLABELSTACK1d,1,3,this->int_totalPageSize)==0) return false;
			pRgnGrow->PHCLABELSTACKPOS = 0;
			pRgnGrow->PHCURGN = new RGN;
			if (!pRgnGrow->PHCURGN)
			{
				v3d_msg("Unable to do:pRgnGrow->PHCURGN = new RGN, program canceled!!!");
				return false;
			}
			pRgnGrow->PHCURGN_head = pRgnGrow->PHCURGN;
			pRgnGrow->TOTALRGNnum = 1;
			if (pRgnGrow->PHCDONEIMG1d) {delete pRgnGrow->PHCDONEIMG1d;pRgnGrow->PHCDONEIMG1d=0;}
			if (pRgnGrow->PHCDONEIMG3d) {delete pRgnGrow->PHCDONEIMG3d;pRgnGrow->PHCDONEIMG3d=0;}

			if (newIntImage3dPairMatlabProtocol(pRgnGrow->PHCDONEIMG3d,pRgnGrow->PHCDONEIMG1d,pRgnGrow->ImgDep,pRgnGrow->ImgHei,pRgnGrow->ImgWid)==0) return false;
			for(int j=minlevel;j<=maxlevel;j++)
			{
				int depk, colj, rowi;
				BYTE * PHCDONEIMG1d = pRgnGrow->PHCDONEIMG1d;
				UBYTE * quantImg1d = pRgnGrow->quantImg1d;
				BYTE *** flagImg = pRgnGrow->PHCDONEIMG3d;
				for (V3DLONG tmpi=0; tmpi<this->int_totalPageSize; tmpi++)
				{
					PHCDONEIMG1d[tmpi] = (quantImg1d[tmpi]==(UBYTE)j)?1:0;
				}
				pRgnGrow->PHCURLABEL = 0;
				for(depk=0; depk<pRgnGrow->ImgDep; depk++)
				{
					for(colj=0; colj<pRgnGrow->ImgHei; colj++)
					{
						for(rowi=0; rowi<pRgnGrow->ImgWid; rowi++)
						{
							if (flagImg[depk][colj][rowi]==1)
							{
								pRgnGrow->IFINCREASELABEL = 1;
								pRgnGrow->PHCURLABEL++;
								pRgnGrow->PHCLABELSTACKPOS = 0;
								pRgnGrow->PHCLABELSTACK3d[0][0][pRgnGrow->PHCLABELSTACKPOS] = depk;
								pRgnGrow->PHCLABELSTACK3d[0][1][pRgnGrow->PHCLABELSTACKPOS] = colj;
								pRgnGrow->PHCLABELSTACK3d[0][2][pRgnGrow->PHCLABELSTACKPOS] = rowi;
								pRgnGrow->PHCURGNPOS = new POS;
								if (pRgnGrow->PHCURGNPOS==0)
								{
									v3d_msg("Fail to allocate memory for PHCURGNPOS, program canceled!!!");
									return false;
								}
								pRgnGrow->PHCURGNPOS_head = pRgnGrow->PHCURGNPOS;
								pRgnGrow->TOTALPOSnum = 1;
								while(1)
								{
									pRgnGrow->IFINCREASELABEL = 1;
									V3DLONG posbeg = pRgnGrow->PHCLABELSTACKPOS;
									V3DLONG mypos = posbeg;
									while (mypos>=0)
									{
										pRgnGrow->STACKCNT = 0;
										int curdep = pRgnGrow->PHCLABELSTACK3d[0][0][mypos];
										int curcol = pRgnGrow->PHCLABELSTACK3d[0][1][mypos];
										int currow = pRgnGrow->PHCLABELSTACK3d[0][2][mypos];
										if (flagImg[curdep][curcol][currow]==1)
										{
											rgnfindsub(currow,curcol,curdep,0,1,pRgnGrow);
										}
										else if(flagImg[curdep][curcol][currow]==-1)
										{
											rgnfindsub(currow,curcol,curdep,0,0,pRgnGrow);
										}
										V3DLONG posend = pRgnGrow->PHCLABELSTACKPOS;
										if (posend>posbeg)
										{mypos = pRgnGrow->PHCLABELSTACKPOS;}
										else
										{mypos = mypos-1;}
										posbeg = posend;
									}
									if (pRgnGrow->IFINCREASELABEL==1)
										break;
								}
								pRgnGrow->PHCURGN->layer = j;
								pRgnGrow->PHCURGN->no = pRgnGrow->PHCURLABEL;
								pRgnGrow->PHCURGN->poslist = pRgnGrow->PHCURGNPOS_head;
								pRgnGrow->PHCURGN->poslistlen = pRgnGrow->TOTALPOSnum;
								pRgnGrow->TOTALPOSnum = 0;
								pRgnGrow->PHCURGN->next = new RGN;
								if(pRgnGrow->PHCURGN->next==0)
								{
									v3d_msg("fail to do --> pRgnGrow->PHCURGN->next = new RGN, program canceled!!!");
									return false;
								}
								pRgnGrow->PHCURGN = pRgnGrow->PHCURGN->next;
								pRgnGrow->TOTALRGNnum++;
							}
						}
					}
				}
			}

			STCL *staRegion = new STCL;
			STCL *staRegion_begin = staRegion;
			RGN *curRgn = pRgnGrow->PHCURGN_head;
			V3DLONG nrgncopied = 0;
			std::vector<STCL> stclList;
			while(curRgn && curRgn->next)
			{
				staRegion->no = curRgn->no;
				staRegion->count = 0;
				POS * curPos = curRgn->poslist;
				V3DLONG count = 0;
				staRegion->desposlist = new V3DLONG [curRgn->poslistlen-1];
				while(curPos && curPos->next)
				{
					staRegion->desposlist[count++] = curPos->pos;
					curPos = curPos->next;
				}
				staRegion->count = count;
				if(count<int_fragmentSizeThreshold)
				{
					nrgncopied++;
					curRgn = curRgn->next;
					for (int i=0;i<count;i++)
					{
						Image1D_mask[staRegion->desposlist[i]] = 1; //invalid;
						Image1D_main[staRegion->desposlist[i]] = 0;
					}
					continue; 
				}
				stclList.push_back(*staRegion);
				curRgn = curRgn->next;
				staRegion->next = new STCL;
				staRegion = staRegion->next;
				nrgncopied++;
			}
			V3DLONG length;
			V3DLONG n_rgn = stclList.size();
			
			int_resultType = 1;
			float *pRGCL = NULL;
			pRGCL = new float [this->int_totalPageSize];
			memset(pRGCL, 0, sizeof(float)*this->int_totalPageSize);
			this->count_grownRegions  = 0;
			for(int ii=0; ii<n_rgn; ii++)
			{
				length = stclList.at(ii).count;
				V3DLONG *cutposlist = stclList.at(ii).desposlist;
				this->vctList_grownRegions.push_back(cutposlist);
				float scx=0,scy=0,scz=0,si=0;
				for(int i=0; i<length; i++)
				{
					pRGCL[ cutposlist[i] ] = (float)ii + 1.0;
					float cv = Image1D_original[ cutposlist[i] ];
					V3DLONG idx = cutposlist[i];
					V3DLONG k1 = idx/(int_xDim*int_yDim);
					V3DLONG j1 = (idx - k1*int_xDim*int_yDim)/int_xDim;
					V3DLONG i1 = idx - k1*int_xDim*int_yDim - j1*int_xDim;
					scz += k1*cv;
					scy += j1*cv;
					scx += i1*cv;
					si += cv;
				}
				if (si>0)
				{
					V3DLONG ncx = scx/si + 0.5 +1;
					V3DLONG ncy = scy/si + 0.5 +1;
					V3DLONG ncz = scz/si + 0.5 +1;
					LocationSimple pp(ncx, ncy, ncz);
					vct_grownRegionsVolumes.push_back(length);
					this->count_grownRegions++;
				}
			}
			Image1D_resultFloat = pRGCL;


			
			if (pRgnGrow->quantImg1d) {delete pRgnGrow->quantImg1d;pRgnGrow->quantImg1d=0;}
			if (pRgnGrow->quantImg3d) {delete pRgnGrow->quantImg3d;pRgnGrow->quantImg3d=0;}
			if (pRgnGrow->PHCLABELSTACK1d) {delete pRgnGrow->PHCLABELSTACK1d;pRgnGrow->PHCLABELSTACK1d=0;}
			if (pRgnGrow->PHCLABELSTACK3d) {delete pRgnGrow->PHCLABELSTACK3d;pRgnGrow->PHCLABELSTACK3d=0;}
			if (pRgnGrow->PHCDONEIMG3d) {delete pRgnGrow->PHCDONEIMG3d;pRgnGrow->PHCDONEIMG3d=0;}
			if (pRgnGrow->PHCDONEIMG1d) {delete pRgnGrow->PHCDONEIMG1d;pRgnGrow->PHCDONEIMG1d=0;}
			return true;
		}

		V3DLONG removeSingleVoxel()
		{
			V3DLONG count_voxelRemoved = 0;
			
			for(V3DLONG k = 0; k < int_zDim; k++)
			{
				V3DLONG idxk = k*this->int_offsetByZ;
				for(V3DLONG j = 0;  j < int_yDim; j++)
				{
					V3DLONG idxj = idxk + j*this->int_offsetByY;
					for(V3DLONG i = 0, idx = idxj; i < int_xDim;  i++, idx++)
					{
						if(i==0 || i==int_xDim-1 || j==0 || j==int_yDim-1 || k==0 || k==int_zDim-1)
							continue;
						if(Image1D_main[idx])
						{
							bool one_point = true;
							for(V3DLONG ineighbor=0; ineighbor<const_int_count_neighbors; ineighbor++)
							{
								V3DLONG n_idx = idx + vct_neighbors[ineighbor];
								if (n_idx>0 && n_idx<this->int_totalPageSize)
								{
									if(Image1D_main[n_idx])
									{
										one_point = false;
										break;
									}
								}
							}
							if(one_point==true)
							{
								count_voxelRemoved = count_voxelRemoved + 1;
								Image1D_main[idx] = 0;
								Image1D_mask[idx] = 1; //invalid;
							}
						}
					}
				}
			}
			return count_voxelRemoved;
		}
		
		
	#pragma region private
	private: 
		class POS
		{
		public:
			V3DLONG pos;
			V3DLONG order;
			POS * next;
			POS()
			{
				pos = -1;order=-1;
				next = 0;
			}
			~POS()
			{
				
			}
		};

		class RGN
		{
		public:
			V3DLONG layer;
			V3DLONG no;
			POS *poslist;
			V3DLONG poslistlen;
			RGN * next;
			RGN()
			{
				layer=no=-1;
				poslistlen=0;poslist=0;
				next=0;
			}
			~RGN()
			{
				layer=no=-1;
				poslistlen = 0;
			}
		};

		//statistics of count of labeling
		class STCL
		{
		public:

			V3DLONG count;
			V3DLONG no;
			V3DLONG *desposlist;
			STCL *next;

			STCL()
			{
				count=no=-1;
				next=0;
			}
			~STCL()
			{
				count=no=-1;
			}
		};

		//function of swap
		template <class T>
		void swap (T& x, T& y)
		{
			T tmp = x;	x = y; y = tmp;
		}

		//function of quickSort
		template <class T>
		void quickSort(T a[], int l, int r)
		{
			if(l>=r) return;
			int i = l;
			int j = r+1;

			T pivot = a[l];
			while(true)
			{
				do{ i = i+1; } while(a[i]>pivot);
				do{ j = j-1; } while(a[j]<pivot);
				if(i >= j) break;
				swap(a[i], a[j]);
			}
			a[l] = a[j];
			a[j] = pivot;
			quickSort(a, l, j-1);
			quickSort(a, j+1, r);
		}

		//memory management
		template <class T> int newIntImage3dPairMatlabProtocol(T *** & img3d,T * & img1d, V3DLONG imgdep, V3DLONG imghei,V3DLONG imgwid)
		{
			V3DLONG totalpxlnum = imghei*imgwid*imgdep;
			try
			{
				img1d = new T [totalpxlnum];
				img3d = new T ** [imgdep];

				V3DLONG i,j;

				for (i=0;i<imgdep;i++)
				{
					img3d[i] = new T * [imghei];
					for(j=0; j<imghei; j++)
						img3d[i][j] = img1d + i*imghei*imgwid + j*imgwid;

				}

				memset(img1d, 0, sizeof(T)*totalpxlnum);
			}
			catch(...)
			{
				if (img1d) {delete img1d;img1d=0;}
				if (img3d) {delete img3d;img3d=0;}
				printf("Fail to allocate mem in newIntImage2dPairMatlabProtocal()!");
				return 0; //fail
			}
			return 1; //succeed
		}

		class RgnGrow3dClass // region growing class
		{
		public:
			RgnGrow3dClass()
			{
				ImgWid = 0, ImgHei = 0, ImgDep = 0;
				quantImg1d=0; quantImg3d=0;
				PHCDONEIMG3d = 0, PHCDONEIMG1d = 0;

				STACKCNT = -1, MAXSTACKSIZE = 16, IFINCREASELABEL=-1, PHCURLABEL=-1;
				PHCLABELSTACK3d = 0, PHCLABELSTACK1d = 0;
				PHCLABELSTACKPOS = 0;

				PHCURGNPOS = 0, PHCURGNPOS_head = 0;
				PHCURGN = 0, PHCURGN_head = 0;
				TOTALPOSnum = 0, TOTALRGNnum = 0;
			}

			~RgnGrow3dClass()
			{
				if (quantImg1d) {delete quantImg1d;quantImg1d=0;}
				if (quantImg3d) {delete quantImg3d;quantImg3d=0;}
				if (PHCLABELSTACK1d) {delete PHCLABELSTACK1d;PHCLABELSTACK1d=0;}
				if (PHCLABELSTACK3d) {delete PHCLABELSTACK3d;PHCLABELSTACK3d=0;}
				if (PHCDONEIMG1d) {delete PHCDONEIMG1d;PHCDONEIMG1d=0;}
				if (PHCDONEIMG3d) {delete PHCDONEIMG3d;PHCDONEIMG3d=0;}


				ImgWid = 0, ImgHei = 0, ImgDep = 0;

				STACKCNT = -1, MAXSTACKSIZE = 16, IFINCREASELABEL=-1, PHCURLABEL=-1;
				PHCLABELSTACKPOS = 0;

				PHCURGN = PHCURGN_head;
				for(V3DLONG i=0;i<TOTALRGNnum;i++)
				{
					RGN * pnextRgn = 0;
					if (PHCURGN)
					{
						pnextRgn = PHCURGN->next;
						PHCURGNPOS = PHCURGN->poslist;
						for(V3DLONG j=0;j<PHCURGN->poslistlen;j++)
						{
							POS *pnextPos = 0;
							if (PHCURGNPOS)
							{
								pnextPos = PHCURGNPOS->next;
								delete PHCURGNPOS;
							}
							PHCURGNPOS = pnextPos;
						}
						delete PHCURGN;
					}
					PHCURGN = pnextRgn;
				}
				TOTALPOSnum = 0, TOTALRGNnum = 0;
			}

		public:
			V3DLONG ImgWid, ImgHei, ImgDep;
			UBYTE * quantImg1d,  *** quantImg3d;
			BYTE *** PHCDONEIMG3d, * PHCDONEIMG1d;

			int STACKCNT;
			int MAXSTACKSIZE;
			int IFINCREASELABEL;
			V3DLONG PHCURLABEL;
			int ***PHCLABELSTACK3d, * PHCLABELSTACK1d;
			V3DLONG PHCLABELSTACKPOS;

			POS * PHCURGNPOS, * PHCURGNPOS_head;
			RGN * PHCURGN, * PHCURGN_head;

			V3DLONG TOTALPOSnum, TOTALRGNnum;
		};

		//generating an int image for any input image type
		template <class T> void copyvecdata(T * srcdata, V3DLONG len, UBYTE * desdata, int& nstate, UBYTE &minn, UBYTE &maxx)
		{
			if(!srcdata || !desdata)
			{
				printf("NULL pointers in copyvecdata()!\n");
				return;
			}

			V3DLONG i;
			//copy data
			if (srcdata[0]>0)
				maxx = minn = int(srcdata[0]+0.5);
			else
				maxx = minn = int(srcdata[0]-0.5);

			int tmp;
			double tmp1;
			for (i=0;i<len;i++)
			{
				tmp1 = double(srcdata[i]);
				tmp = (tmp1>0)?(int)(tmp1+0.5):(int)(tmp1-0.5);//round to integers
				minn = (minn<tmp)?minn:tmp;
				maxx = (maxx>tmp)?maxx:tmp;
				desdata[i] = (UBYTE)tmp;
			}
			maxx = (UBYTE)maxx;
			minn = (UBYTE)minn;
			//return the #state
			nstate = (maxx-minn+1);
			return;
		}

		void rgnfindsub(int rowi,int colj, int depk, int direction,int stackinc, RgnGrow3dClass * pRgnGrow)
		{
			if (pRgnGrow->STACKCNT >= pRgnGrow->MAXSTACKSIZE)
			{
				if (pRgnGrow->IFINCREASELABEL != 0)
					pRgnGrow->IFINCREASELABEL = 0;
				return;
			}

			BYTE *** flagImg = pRgnGrow->PHCDONEIMG3d;
			int ImgWid = pRgnGrow->ImgWid;
			int ImgHei = pRgnGrow->ImgHei;
			int ImgDep = pRgnGrow->ImgDep;

			if (stackinc==1)
			{
				pRgnGrow->PHCLABELSTACK3d[0][0][pRgnGrow->PHCLABELSTACKPOS] = depk;
				pRgnGrow->PHCLABELSTACK3d[0][1][pRgnGrow->PHCLABELSTACKPOS] = colj;
				pRgnGrow->PHCLABELSTACK3d[0][2][pRgnGrow->PHCLABELSTACKPOS] = rowi;

				pRgnGrow->STACKCNT++;
				pRgnGrow->PHCLABELSTACKPOS++;

				flagImg[depk][colj][rowi] = -1;
				
				//set the current pos location and return the
				if (pRgnGrow->PHCURGNPOS)
				{
					pRgnGrow->PHCURGNPOS->pos = (V3DLONG) depk*(pRgnGrow->ImgHei * pRgnGrow->ImgWid) + colj*(pRgnGrow->ImgWid) + rowi; //
					pRgnGrow->PHCURGNPOS->next = new POS;
					if (pRgnGrow->PHCURGNPOS->next==0)
					{printf("Fail to do: pRgnGrow->PHCURGNPOS->next = new POS;");}
					pRgnGrow->PHCURGNPOS = pRgnGrow->PHCURGNPOS->next;
					pRgnGrow->TOTALPOSnum++;
				}
				else
				{
					printf("PHCURGNPOS is null!!\n");
				}
			}
			else //%if stackinc==0,
			{
				flagImg[depk][colj][rowi] = -2;
			}
			// % search 26 direction orders
			// 1
			if (rowi>0 && flagImg[depk][colj][rowi-1]==1)
				rgnfindsub(rowi-1,colj,depk,1,1,pRgnGrow);
			// 2
			if (rowi<ImgWid-1 && flagImg[depk][colj][rowi+1]==1)
				rgnfindsub(rowi+1,colj,depk,1,1,pRgnGrow);
			// 3
			if (colj>0 && flagImg[depk][colj-1][rowi]==1)
				rgnfindsub(rowi,colj-1,depk,1,1,pRgnGrow);
			// 4
			if (colj<ImgHei-1 && flagImg[depk][colj+1][rowi]==1)
				rgnfindsub(rowi,colj+1,depk,1,1,pRgnGrow);
			// 5
			if (depk>0 && flagImg[depk-1][colj][rowi]==1)
				rgnfindsub(rowi,colj,depk-1,1,1,pRgnGrow);
			// 6
			if (depk<ImgDep-1 && flagImg[depk+1][colj][rowi]==1)
				rgnfindsub(rowi,colj,depk+1,1,1,pRgnGrow);
			// 7
			if (rowi>0 && colj>0 && flagImg[depk][colj-1][rowi-1]==1)
				rgnfindsub(rowi-1,colj-1,depk,1,1,pRgnGrow);
			// 8
			if (rowi<ImgWid-1 && colj>0 && flagImg[depk][colj-1][rowi+1]==1)
				rgnfindsub(rowi+1,colj-1,depk,1,1,pRgnGrow);
			// 9
			if (rowi>0 && colj<ImgHei-1 && flagImg[depk][colj+1][rowi-1]==1)
				rgnfindsub(rowi-1,colj+1,depk,1,1,pRgnGrow);
			// 10
			if (rowi>ImgWid && colj<ImgHei-1 && flagImg[depk][colj+1][rowi+1]==1)
				rgnfindsub(rowi+1,colj+1,depk,1,1,pRgnGrow);
			// 11
			if (rowi>0 && depk>0 && flagImg[depk-1][colj][rowi-1]==1)
				rgnfindsub(rowi-1,colj,depk-1,1,1,pRgnGrow);
			// 12
			if (rowi<ImgWid-1 && depk>0 && flagImg[depk-1][colj][rowi+1]==1)
				rgnfindsub(rowi+1,colj,depk-1,1,1,pRgnGrow);
			// 13
			if (rowi>0 && depk<ImgDep-1 && flagImg[depk+1][colj][rowi-1]==1)
				rgnfindsub(rowi-1,colj,depk+1,1,1,pRgnGrow);
			// 14
			if (rowi<ImgWid-1 && depk<ImgDep-1 && flagImg[depk+1][colj][rowi+1]==1)
				rgnfindsub(rowi+1,colj,depk+1,1,1,pRgnGrow);
			// 15
			if (colj>0 && depk>0 && flagImg[depk-1][colj-1][rowi]==1)
				rgnfindsub(rowi,colj-1,depk-1,1,1,pRgnGrow);
			// 16
			if (colj<ImgHei-1 && depk>0 && flagImg[depk-1][colj+1][rowi]==1)
				rgnfindsub(rowi,colj+1,depk-1,1,1,pRgnGrow);
			// 17
			if (colj>0 && depk<ImgDep-1 && flagImg[depk+1][colj-1][rowi]==1)
				rgnfindsub(rowi,colj-1,depk+1,1,1,pRgnGrow);
			// 18
			if (colj<ImgHei-1 && depk<ImgDep-1 && flagImg[depk+1][colj+1][rowi]==1)
				rgnfindsub(rowi,colj+1,depk+1,1,1,pRgnGrow);
			// 19
			if (rowi>0 && colj>0 && depk>0 && flagImg[depk-1][colj-1][rowi-1]==1)
				rgnfindsub(rowi-1,colj-1,depk-1,1,1,pRgnGrow);
			// 20
			if (rowi<ImgWid-1 && colj>0 && depk>0 && flagImg[depk-1][colj-1][rowi+1]==1)
				rgnfindsub(rowi+1,colj-1,depk-1,1,1,pRgnGrow);
			// 21
			if (rowi>0 && colj<ImgHei-1 && depk>0 && flagImg[depk-1][colj+1][rowi-1]==1)
				rgnfindsub(rowi-1,colj+1,depk-1,1,1,pRgnGrow);
			// 22
			if (rowi>0 && colj>0 && depk<ImgDep-1 && flagImg[depk+1][colj-1][rowi-1]==1)
				rgnfindsub(rowi-1,colj-1,depk+1,1,1,pRgnGrow);
			// 23
			if (rowi<ImgWid-1 && colj<ImgHei-1 && depk>0 && flagImg[depk-1][colj+1][rowi+1]==1)
				rgnfindsub(rowi+1,colj+1,depk-1,1,1,pRgnGrow);
			// 24
			if (rowi<ImgWid-1 && colj>0 && depk<ImgDep-1 && flagImg[depk+1][colj-1][rowi+1]==1)
				rgnfindsub(rowi+1,colj-1,depk+1,1,1,pRgnGrow);
			// 25
			if (rowi>0 && colj<ImgHei-1 && depk<ImgDep-1 && flagImg[depk+1][colj+1][rowi-1]==1)
				rgnfindsub(rowi-1,colj+1,depk+1,1,1,pRgnGrow);
			// 26
			if (rowi<ImgWid-1 && colj<ImgHei-1 && depk<ImgDep-1 && flagImg[depk+1][colj+1][rowi+1]==1)
				rgnfindsub(rowi+1,colj+1,depk+1,1,1,pRgnGrow);
			return;
		}
		#pragma endregion
};
#pragma endregion

#pragma region "function: func_Image1DVisualization"
void func_Image1DVisualization(unsigned char* Image1D_input, V3DLONG int_xDim, V3DLONG int_yDim, V3DLONG int_zDim, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName)
{
	Image4DSimple Image4DSimple_temp;
	//V3DLONG count_totalPageSize = int_xDim*int_zDim*int_zDim;
	Image4DSimple_temp.setData((unsigned char*)Image1D_input, int_xDim, int_yDim, int_zDim, int_channelDim, V3D_UINT8);
	v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
	V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
	V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
	V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
}

void func_regionListVisualization(vector<vector<V3DLONG>> vctList_regionsInput, V3DLONG int_xDim, V3DLONG int_yDim, V3DLONG int_zDim, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName)
{
	Image4DSimple Image4DSimple_temp;
	V3DLONG count_totalPageSize = int_xDim*int_yDim*int_zDim;
	unsigned char* Image1D_tempConverted;
	if (count_totalPageSize > const_int_floatCriteria)
	{
		float* Image1D_tempFloat = new float [count_totalPageSize];
		for (V3DLONG i=0;i<count_totalPageSize;i++)
		{
			Image1D_tempFloat[i] = 0;
		}
		for (V3DLONG idx_region=0;idx_region<vctList_regionsInput.size();idx_region++)
		{
			for (V3DLONG idx_voxel=0;idx_voxel<vctList_regionsInput[idx_region].size();idx_voxel++)
			{
				Image1D_tempFloat[vctList_regionsInput[idx_region][idx_voxel]] = idx_region + 1;
			}
		}
		Image1D_tempConverted = subFunc_Image1DTypeConversion(Image1D_tempFloat, count_totalPageSize);
		Image4DSimple_temp.setData((unsigned char*)Image1D_tempConverted, int_xDim, int_yDim, int_zDim, int_channelDim, V3D_UINT8);
		v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
		V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
		V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
	else if (count_totalPageSize > const_int_longCriteria)
	{
		unsigned short* Image1D_tempShort = new unsigned short [count_totalPageSize];
		for (V3DLONG i=0;i<count_totalPageSize;i++)
		{
			Image1D_tempShort[i] = 0;
		}
		for (V3DLONG idx_region=0;idx_region<vctList_regionsInput.size();idx_region++)
		{
			for (V3DLONG idx_voxel=0;idx_voxel<vctList_regionsInput[idx_region].size();idx_voxel++)
			{
				Image1D_tempShort[vctList_regionsInput[idx_region][idx_voxel]] = idx_region + 1;
			}
		}
		Image1D_tempConverted = subFunc_Image1DTypeConversion(Image1D_tempShort, count_totalPageSize);
		Image4DSimple_temp.setData((unsigned char*)Image1D_tempConverted, int_xDim, int_yDim, int_zDim, int_channelDim, V3D_UINT8);
		v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
		V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
		V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
	else
	{
		unsigned char* Image1D_temp = new unsigned char [count_totalPageSize];
		for (V3DLONG i=0;i<count_totalPageSize;i++)
		{
			Image1D_temp[i] = 0;
		}
		for (V3DLONG idx_region=0;idx_region<vctList_regionsInput.size();idx_region++)
		{
			for (V3DLONG idx_voxel=0;idx_voxel<vctList_regionsInput[idx_region].size();idx_voxel++)
			{
				Image1D_temp[vctList_regionsInput[idx_region][idx_voxel]] = idx_region + 1;
			}
		}
		Image4DSimple_temp.setData((unsigned char*)Image1D_temp, int_xDim, int_yDim, int_zDim, int_channelDim, V3D_UINT8);
		v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
		V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
		V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
}

void func_regionVisualization(vector<V3DLONG> vct_regionInput, V3DLONG int_xDim, V3DLONG int_yDim, V3DLONG int_zDim, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName)
{
	Image4DSimple Image4DSimple_temp;
	V3DLONG count_totalPageSize = int_xDim*int_yDim*int_zDim;
	unsigned char* Image1D_tempConverted;
	if (count_totalPageSize > const_int_floatCriteria)
	{
		float* Image1D_tempFloat = new float [count_totalPageSize];
		for (V3DLONG i=0;i<count_totalPageSize;i++)
		{
			Image1D_tempFloat[i] = 0;
		}
		for (V3DLONG idx_voxel=0;idx_voxel<vct_regionInput.size();idx_voxel++)
		{
			Image1D_tempFloat[vct_regionInput[idx_voxel]] = 255;
		}
		Image1D_tempConverted = subFunc_Image1DTypeConversion(Image1D_tempFloat, count_totalPageSize);
		Image4DSimple_temp.setData((unsigned char*)Image1D_tempConverted, int_xDim, int_yDim, int_zDim, int_channelDim, V3D_UINT8);
		v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
		V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
		V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
	else if (count_totalPageSize > const_int_longCriteria)
	{
		unsigned short* Image1D_tempShort = new unsigned short [count_totalPageSize];
		for (V3DLONG i=0;i<count_totalPageSize;i++)
		{
			Image1D_tempShort[i] = 0;
		}
		for (V3DLONG idx_voxel=0;idx_voxel<vct_regionInput.size();idx_voxel++)
		{
			Image1D_tempShort[vct_regionInput[idx_voxel]] = 255;
		}
		Image1D_tempConverted = subFunc_Image1DTypeConversion(Image1D_tempShort, count_totalPageSize);
		Image4DSimple_temp.setData((unsigned char*)Image1D_tempConverted, int_xDim, int_yDim, int_zDim, int_channelDim, V3D_UINT8);
		v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
		V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
		V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
	else
	{
		unsigned char* Image1D_temp = new unsigned char [int_xDim*int_yDim*int_zDim];
		for (V3DLONG i=0;i<count_totalPageSize;i++)
		{
			Image1D_temp[i] = 0;
		}
		for (V3DLONG idx_voxel=0;idx_voxel<vct_regionInput.size();idx_voxel++)
		{
			Image1D_temp[vct_regionInput[idx_voxel]] = 255;
		}
		Image4DSimple_temp.setData((unsigned char*)Image1D_temp, int_xDim, int_yDim, int_zDim, int_channelDim, V3D_UINT8);
		v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
		V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
		V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
}
#pragma endregion

#pragma region "Segmentation interface"
bool func_interface_Segmentation(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
{
	ofstream ofstream_log;
	ofstream_log.open ("log_func_interface_Segmentation.txt");

	//get current image;
    v3dhandle v3dhandle_currentWindow = V3DPluginCallback2_currentCallback.currentImageWindow();
    //cancels if no image loaded;
    if (!v3dhandle_currentWindow)
    {
        v3d_msg("You have not loaded any image or the image is corrupted, program canceled!!!");
		ofstream_log.close();
        return false;
    }
    //try to pull the data as an Image4DSimple (3D+channel);
    Image4DSimple* Image4DSimple_current = V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
	if (!Image4DSimple_current)
	{
		v3d_msg("You have not loaded any image or the image is corrupted, program canceled!!!");
		ofstream_log.close();
        return false;
	}
	V3DLONG count_totalBytes = Image4DSimple_current->getTotalBytes();
	if (count_totalBytes < 1)
	{
		v3d_msg("You have not loaded any image or the image is corrupted, program canceled!!!");
		ofstream_log.close();
        return false;
	}
	ImagePixelType ImagePixelType_current = Image4DSimple_current->getDatatype();
	if(ImagePixelType_current != V3D_UINT8)
	{
		v3d_msg("Currently this program only support 8-bit data, program canceled!!!");
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"Voxel count (by couting bytes): "<<(count_totalBytes/sizeof(Image4DSimple_current[0]))<<endl;
	//sets data into 1D array, note: size is count_totalVoxelsCurrent;
    unsigned char* Image1D_current = Image4DSimple_current->getRawData();
	//get the currenlty-opened file name
    QString QString_fileNameCurrent = V3DPluginCallback2_currentCallback.getImageName(v3dhandle_currentWindow);
    //defining dimensions
    V3DLONG int_xDim = Image4DSimple_current->getXDim();
    V3DLONG int_yDim = Image4DSimple_current->getYDim();
    V3DLONG int_zDim = Image4DSimple_current->getZDim();
    V3DLONG int_channelDim = Image4DSimple_current->getCDim();
    

	//get current LandmarkList (one way of defining exemplars);
    LandmarkList LandmarkList_userDefined = V3DPluginCallback2_currentCallback.getLandmark(v3dhandle_currentWindow);
	V3DLONG count_userDefinedLandmarkList = LandmarkList_userDefined.count();
    //get current NeuronTreeList (i.e. SWC list, another way of defining exemplars);
    QList<NeuronTree> * SWCList_current = V3DPluginCallback2_currentCallback.getHandleNeuronTrees_3DGlobalViewer(v3dhandle_currentWindow);
	V3DLONG count_SWCList = 0;
	if (!SWCList_current)
	{
		//no SWC loaded, do nothing to avoid pointer to NULL;
	}
	else
	{
		count_SWCList = SWCList_current->count();
	}

    //check data availability;
	LandmarkList LandmarkList_current;
	V3DLONG count_currentLandmarkList = -1;
	if ((count_SWCList<1) && (count_userDefinedLandmarkList<1)) //Both of the lists are empty;
    {
        v3d_msg("You have not defined any landmarks or swc structure to run the segmenation, program canceled!!!");
		ofstream_log.close();
        return false;
    }
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList>0)) //Both of the lists are not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		subFunc_NeuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList<1)) //Only SWCList_current is not empty;
	{
		subFunc_NeuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}
	if (count_userDefinedLandmarkList>0) //Only LandmarkList_userDefined is not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}

	//Open dialogMain1 window, get paramters;
    dialogMain dialogMain1(V3DPluginCallback2_currentCallback, QWidget_parent, int_channelDim);
    if (dialogMain1.exec()!=QDialog::Accepted)
    {
		ofstream_log.close();
        return false;
    }

    V3DLONG int_channel = dialogMain1.int_channel;
    double double_threshold = dialogMain1.double_threshold;
    if (double_threshold>const_double_maxPixelIntensity)
	{
		v3d_msg("Please provide a valid threshold, program canceled!!!"); 
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"Begin to initialization class_segmentationMain!!!"<<endl;
	class_segmentationMain segmentationMain1(double_threshold, Image1D_current, int_xDim, int_yDim, int_zDim, int_channel, LandmarkList_current);
	ofstream_log<<"class_segmentationMain initialization succeed!!!"<<endl;
	ofstream_log<<"Current dimension: int_xDim: "<<segmentationMain1.int_xDim<<", int_yDim: "<<segmentationMain1.int_yDim<<", int_zDim: "<<segmentationMain1.int_zDim<<endl;
	ofstream_log<<"Current dimension: int_totalPageSize: "<<segmentationMain1.int_totalPageSize<<endl;
	ofstream_log<<"Current channel: "<<segmentationMain1.int_channel<<endl;
	ofstream_log<<"Threshold: "<<segmentationMain1.double_threshold<<endl;
	ofstream_log<<"count_exemplerRegionVoxels: "<<segmentationMain1.count_exemplerRegionVoxels<<endl;
	ofstream_log<<"Number of invalid exemplers: "<<segmentationMain1.vct_invalidExempler.size()<<endl;
	
	ofstream_log<<"Begin to segmentationMain1.threshold2BinaryForExemplerRegion()!!!"<<endl;
	V3DLONG count_totalWhite = segmentationMain1.threshold2BinaryForExemplerRegion();
	ofstream_log<<"segmentationMain1.threshold2BinaryForExemplerRegion() succeed, total white voxels: "<<count_totalWhite<<"!!!"<<endl;

	ofstream_log<<"Begin to segmentationMain1.analyzeExemplerRegion()!!!"<<endl;
	segmentationMain1.analyzeExemplerRegion(); //NOTE: if you want this function to run correctly, please do it AFTER threshold2BinaryForExemplerRegion() and BEFORE threshold2BinaryForMain();
	ofstream_log<<"segmentationMain1.analyzeExemplerRegion() succeed, int_minExemplerRegionVolume: "<<segmentationMain1.int_minExemplerRegionVolume<<"!!!"<<endl;

	ofstream_log<<"Begin to segmentationMain1.threshold2BinaryForMain()!!!"<<endl;
	count_totalWhite = segmentationMain1.threshold2BinaryForMain();
	ofstream_log<<"segmentationMain1.threshold2BinaryForMain() succeed, total white voxels: "<<count_totalWhite<<"!!!"<<endl;

	ofstream_log<<"Begin to segmentationMain1.removeSingleVoxel()!!!"<<endl;
	V3DLONG count_voxelRemoved = segmentationMain1.removeSingleVoxel();
	ofstream_log<<"segmentationMain1.removeSingleVoxel() succeed, "<<count_voxelRemoved<<" removed!!!"<<endl;

	ofstream_log<<"Begin to convert Image1D_exemplerRegion"<<endl;
	unsigned char* Image1D_resultConverted = subFunc_Image1DTypeConversion(segmentationMain1.Image1D_exemplerRegion, segmentationMain1.int_totalPageSize);
	ofstream_log<<"Converting Image1D_exemplerRegion succeed!!!"<<endl;
	ofstream_log<<"Begin to visualize segmentationMain1.regonGrowOnExempler() results !!!"<<endl;
	func_Image1DVisualization(Image1D_resultConverted,segmentationMain1.int_xDim,segmentationMain1.int_yDim,segmentationMain1.int_zDim,1,V3DPluginCallback2_currentCallback, "Exempler Regions");
	ofstream_log<<"Visualize segmentationMain1.regonGrowOnExempler() succeed !!!"<<endl;
	
	ofstream_log<<"Begin to segmentationMain1.regionGrow()!!!"<<endl;
	if(segmentationMain1.regionGrow())
	{
		ofstream_log<<"segmentationMain1.regionGrow() succeed!!!"<<endl;
		ofstream_log<<"Total regions: "<<segmentationMain1.count_grownRegions<<"!!!"<<endl;
	}
	else
	{
		ofstream_log<<"segmentationMain1.regionGrow() failed!!!"<<endl;
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"Begin to segmentationMain1.regionFitting()!!!"<<endl;
	segmentationMain1.regionFitting();
	ofstream_log<<"segmentationMain1.regionFitting() succeed!!!"<<endl;

	ofstream_log<<"Begin to convert segmentationMain1.Image1D_resultFloat!!!"<<endl;
	Image1D_resultConverted = subFunc_Image1DTypeConversion(segmentationMain1.Image1D_resultFloat, segmentationMain1.int_totalPageSize);
	ofstream_log<<"Converting segmentationMain1.Image1D_resultFloat succeed!!!"<<endl;
	ofstream_log<<"segmentationMain1.Image1D_resultFloat Visualizing!!!"<<endl;
	func_Image1DVisualization(Image1D_resultConverted,segmentationMain1.int_xDim,segmentationMain1.int_yDim,segmentationMain1.int_zDim,1,V3DPluginCallback2_currentCallback, "Segmentation results");
	ofstream_log<<"segmentationMain1.Image1D_resultFloat Visualization succeed!!!"<<endl;	

	ofstream_log.close();
    return true;
}
#pragma endregion

#pragma region "Utility funcions"
bool subFunc_NeuronTree2LandmarkList(const NeuronTree & NeuronTree_input, LandmarkList & LandmarkList_output)
{
    LocationSimple LocationSimple_temp(0,0,0);
    for (V3DLONG i=0;i<NeuronTree_input.listNeuron.size();i++)
    {
        LocationSimple_temp.x = NeuronTree_input.listNeuron.at(i).x;
        LocationSimple_temp.y = NeuronTree_input.listNeuron.at(i).y;
        LocationSimple_temp.z = NeuronTree_input.listNeuron.at(i).z;
        LandmarkList_output.append(LocationSimple_temp);
    }
    return true;
}

unsigned char* subFunc_Image1DTypeConversion(unsigned short *Image1D_input, V3DLONG count_total)
{
	unsigned char* Image1D_Output = new unsigned char [count_total];
	for (V3DLONG i=0;i<count_total;i++)
	{
		unsigned short int_temp = Image1D_input[i];
		Image1D_Output[i] = int_temp;
	}
	return Image1D_Output;
}

unsigned char* subFunc_Image1DTypeConversion(float *Image1D_input, V3DLONG count_total)
{
	unsigned char* Image1D_Output = new unsigned char [count_total];
	for (V3DLONG i=0;i<count_total;i++)
	{
		float int_temp = Image1D_input[i];
		Image1D_Output[i] = int_temp;
	}
	return Image1D_Output;
}
#pragma endregion