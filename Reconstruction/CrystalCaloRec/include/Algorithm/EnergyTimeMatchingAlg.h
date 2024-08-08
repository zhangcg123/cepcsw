#ifndef ETMATCHING_ALG_H
#define ETMATCHING_ALG_H

#include "PandoraPlusDataCol.h"
#include "Tools/Algorithm.h"

#include "TVector3.h"
using namespace PandoraPlus;

class EnergyTimeMatchingAlg: public PandoraPlus::Algorithm{

public: 

  class Factory : public PandoraPlus::AlgorithmFactory
  {
  public:
    PandoraPlus::Algorithm* CreateAlgorithm() const{ return new EnergyTimeMatchingAlg(); }

  };

  EnergyTimeMatchingAlg(){};
  ~EnergyTimeMatchingAlg(){};

  StatusCode ReadSettings( PandoraPlus::Settings& m_settings);
  StatusCode Initialize( PandoraPlusDataCol& m_datacol );
  StatusCode RunAlgorithm( PandoraPlusDataCol& m_datasvc );
  StatusCode ClearAlgorithm(); 

  StatusCode Matching(std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusUCol, 
                      std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusVCol,
                      std::vector<std::shared_ptr<PandoraPlus::Calo3DCluster>>& m_clusterCol );

  StatusCode PatternMatrixCalculation(std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusUCol,
                                      std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusVCol,
                                      vector<vector<int>>& matrix );

  StatusCode Chi2MatrixCalculation( std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusUCol,
                                    std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusVCol,
                                    vector<vector<double>>& matrix, 
                                    std::vector< std::pair<int, int> >& chi2order); 

  StatusCode ParameterMatrixCalculation(const int& M, const int& N, vector<vector<int>>& parMatrix);

  StatusCode SimplityMatrix(vector<vector<int>>& parMatrix, vector<double>& Eij, vector<vector<int>>& pattern);

  double SolveMatrix(const int& M, const int& N, vector<vector<int>>& parMatrix, vector<double>& Eij, std::vector<double>& En_clusters); 

  StatusCode leastSquares(const vector<vector<int>>& A, const vector<double>& b, vector<double>& x);

  vector<vector<double>> GetClusterChi2Map( std::vector<std::vector<const PandoraPlus::Calo1DCluster*>>& barShowerUCol,
                                            std::vector<std::vector<const PandoraPlus::Calo1DCluster*>>& barShowerVCol );

  StatusCode ClusterBuilding( std::vector<std::vector<double>> Ematrix, 
                              std::vector<const PandoraPlus::CaloHalfCluster*>& m_HFClusUCol, 
                              std::vector<const PandoraPlus::CaloHalfCluster*>& m_HFClusVCol, 
                              std::vector<std::shared_ptr<PandoraPlus::Calo3DCluster>>& m_clusterCol);

  StatusCode XYClusterMatchingL0( const PandoraPlus::CaloHalfCluster* m_longiClU,
                                  const PandoraPlus::CaloHalfCluster* m_longiClV,
                                  std::shared_ptr<PandoraPlus::Calo3DCluster>& m_clus );

  StatusCode GetMatchedShowersL0( const PandoraPlus::Calo1DCluster* barShowerU,
                                  const PandoraPlus::Calo1DCluster* barShowerV,
                                  PandoraPlus::Calo2DCluster* outsh );

  StatusCode GetMatchedShowersL1( const PandoraPlus::Calo1DCluster* shower1,
                                  std::vector<const PandoraPlus::Calo1DCluster*>& showerNCol,
                                  std::vector<PandoraPlus::Calo2DCluster*>& outshCol );

  StatusCode ClusterReconnecting(std::vector<std::shared_ptr<PandoraPlus::Calo3DCluster>>& m_clusterCol);


private: 
  std::vector<PandoraPlus::Calo3DCluster*> m_towerCol; 

  std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusUCol;
  std::vector<const PandoraPlus::CaloHalfCluster*> m_HFClusVCol;
  std::vector<std::shared_ptr<PandoraPlus::Calo3DCluster>> m_clusterCol; 

  PandoraPlusDataCol m_bkCol;

};
#endif
