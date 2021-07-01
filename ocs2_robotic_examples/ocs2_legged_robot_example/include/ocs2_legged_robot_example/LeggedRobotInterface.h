/******************************************************************************
Copyright (c) 2021, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

 * Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

// ocs2
#include <ocs2_core/misc/Display.h>
#include <ocs2_mpc/MPC_DDP.h>
#include <ocs2_oc/oc_problem/OptimalControlProblem.h>
#include <ocs2_oc/synchronized_module/SolverSynchronizedModule.h>
#include <ocs2_pinocchio_interface/PinocchioEndEffectorKinematicsCppAd.h>
#include <ocs2_pinocchio_interface/PinocchioInterface.h>
#include <ocs2_robotic_tools/common/RobotInterface.h>

#include <ocs2_centroidal_model/CentroidalModelPinocchioMapping.h>
#include <ocs2_centroidal_model/FactoryFunctions.h>

// ocs2_legged_robot_example
#include "ocs2_legged_robot_example/common/ModelSettings.h"
#include "ocs2_legged_robot_example/initialization/LeggedRobotInitializer.h"
#include "ocs2_legged_robot_example/logic/GaitReceiver.h"
#include "ocs2_legged_robot_example/logic/SwitchedModelModeScheduleManager.h"

/**
 * LeggedRobotInterface class
 * General interface for mpc implementation on the legged robot model
 */
namespace ocs2 {
namespace legged_robot {

class LeggedRobotInterface final : public RobotInterface {
 public:
  LeggedRobotInterface(const std::string& taskFileFolderName, const std::string& targetCommandFile,
                       const ::urdf::ModelInterfaceSharedPtr& urdfTree);

  ~LeggedRobotInterface() override = default;

  const OptimalControlProblem& getOptimalControlProblem() const override { return *problemPtr_; }

  const LeggedRobotInitializer& getInitializer() const override { return *initializerPtr_; }

  std::shared_ptr<ModeScheduleManager> getModeScheduleManagerPtr() const override { return modeScheduleManagerPtr_; }

  std::unique_ptr<MPC_DDP> getMpcPtr() const;

  const ModelSettings& modelSettings() const { return modelSettings_; }
  const ddp::Settings& ddpSettings() const { return ddpSettings_; }
  const mpc::Settings& mpcSettings() const { return mpcSettings_; }
  const rollout::Settings& rolloutSettings() const { return rolloutSettings_; }

  const vector_t& getInitialState() const { return initialState_; }

  PinocchioInterface& getPinocchioInterface() { return *pinocchioInterfacePtr_; }

  const CentroidalModelInfo& getCentroidalModelInfo() const { return centroidalModelInfo_; }

  const RolloutBase& getRollout() const { return *rolloutPtr_; }

  /** Gets the solver synchronized modules */
  const std::vector<std::shared_ptr<SolverSynchronizedModule>>& getSynchronizedModules() const { return solverModules_; };

  std::shared_ptr<SwitchedModelModeScheduleManager> getSwitchedModelModeScheduleManagerPtr() const { return modeScheduleManagerPtr_; }

 private:
  std::shared_ptr<GaitSchedule> loadGaitSchedule(const std::string& taskFile);
  void setupOptimalConrolProblem(const std::string& taskFile, const std::string& targetCommandFile,
                                 const ::urdf::ModelInterfaceSharedPtr& urdfTree);

  std::unique_ptr<StateInputCost> getBaseTrackingCost(const std::string& taskFile, const CentroidalModelInfo& info);
  void initializeInputCostWeight(const std::string& taskFile, const CentroidalModelInfo& info, matrix_t& R);

  std::unique_ptr<StateInputCost> getFrictionConeConstraint(size_t contactPointIndex, scalar_t frictionCoefficient, scalar_t mu,
                                                            scalar_t delta);
  std::unique_ptr<StateInputConstraint> getZeroForceConstraint(size_t contactPointIndex);
  std::unique_ptr<StateInputConstraint> getZeroVelocityConstraint(const EndEffectorKinematics<scalar_t>& eeKinematics,
                                                                  size_t contactPointIndex, bool useAnalyticalGradients);
  std::unique_ptr<StateInputConstraint> getNormalVelocityConstraint(const EndEffectorKinematics<scalar_t>& eeKinematics,
                                                                    size_t contactPointIndex, bool useAnalyticalGradients);

  bool display_;
  ModelSettings modelSettings_;
  ddp::Settings ddpSettings_;
  mpc::Settings mpcSettings_;
  rollout::Settings rolloutSettings_;

  std::unique_ptr<PinocchioInterface> pinocchioInterfacePtr_;
  CentroidalModelInfo centroidalModelInfo_;

  std::shared_ptr<SwitchedModelModeScheduleManager> modeScheduleManagerPtr_;
  std::vector<std::shared_ptr<SolverSynchronizedModule>> solverModules_;

  std::shared_ptr<SolverSynchronizedModule> referenceUpdateModulePtr_;
  std::shared_ptr<CostDesiredTrajectories> referenceTrajectoryPtr_;
  std::unique_ptr<OptimalControlProblem> problemPtr_;

  std::unique_ptr<RolloutBase> rolloutPtr_;
  std::unique_ptr<LeggedRobotInitializer> initializerPtr_;

  vector_t initialState_;
};

}  // namespace legged_robot
}  // namespace ocs2
