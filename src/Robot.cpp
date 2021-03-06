#include <WPILib.h>
#include <iostream>
#include <memory>
#include <string>

#include <IterativeRobot.h>
#include <Drive/DifferentialDrive.h>
#include <SpeedControllerGroup.h>
#include <LiveWindow/LiveWindow.h>
#include <SmartDashboard/SendableChooser.h>
#include <SmartDashboard/SmartDashboard.h>
#include <ADIS16448IMU/ADIS16448_IMU.h>
#include <ctre/phoenix/MotorControl/CAN/WPI_TalonSRX.h>

class Robot: public frc::IterativeRobot {
public:
	void RobotInit() {
		talonTL = new Spark(0);
		talonBL = new Spark(1);
		talonTR = new Spark(2);
		talonBR = new Spark(3);
		LeftIntake = new VictorSP(4);
		RightIntake = new VictorSP(5);
		Elevator = new ctre::phoenix::motorcontrol::can::WPI_TalonSRX(1);
		Pivot = new Talon(6);
		Elevator2 = new Talon(7);
		IntakeJS = new Joystick(1);
		IntakeArm = new Joystick(2);
		XC = new XboxController(0);
		gyro = new ADIS16448_IMU();
		limitTop = new DigitalInput(0);
		limitBottom = new DigitalInput(1);

		talonTL->SetInverted(true);
		talonBL->SetInverted(true);
		leftGroup = new SpeedControllerGroup(*talonTL, *talonBL);

		talonTR->SetInverted(true);
		talonBR->SetInverted(true);
		rightGroup = new SpeedControllerGroup(*talonTR, *talonBR);

		Elevator->SetInverted(true);


		drive = new DifferentialDrive(*leftGroup, *rightGroup);

		timer = new Timer();
		//gyro = new AnalogGyro(1);
		//chooser.AddDefault(autoNameDefault, autoNameDefault);
		//chooser.AddObject(autoNameCustom, autoNameCustom);
		//frc::SmartDashboard::PutData("Auto Modes", &chooser);
	}

	/*
	 * This autonomous (along with the chooser code above) shows how to select
	 * between different autonomous modes using the dashboard. The sendable
	 * chooser code works with the Java SmartDashboard. If you prefer the
	 * LabVIEW Dashboard, remove all of the chooser code and uncomment the
	 * GetString line to get the auto name from the text box below the Gyro.
	 *
	 * You can add additional auto modes by adding additional comparisons to the
	 * if-else structure below with additional strings. If using the
	 * SendableChooser make sure to add them to the chooser code above as well.
	 */
	void AutonomousInit() {
		//autoSelected = chooser.GetSelected();
		// std::string autoSelected = SmartDashboard::GetString("Auto Selector", autoNameDefault);
		//std::cout << "Auto selected: " << autoSelected << std::endl;

		//if (autoSelected == autoNameCustom) {
			// Custom Auto goes here
		//} else {
			// Default Auto goes here
		//}
		timer->Start();
	}

	void AutonomousPeriodic() {
		//if (autoSelected == autoNameCustom) {
			// Custom Auto goes here
		//} else {
			// Default Auto goes here
		//}

		if(timer->Get() < 15) {
			//float angle = gyro->GetAngle();
			//float Kp = 0.01;
			drive->ArcadeDrive(-0.5, 0); //-angle * Kp);
		} else {
			drive->ArcadeDrive(0, 0);
			timer->Stop();
		}



	}

	void TeleopInit() {

	}

	void TeleopPeriodic() {
		drive->ArcadeDrive(XC->GetY(GenericHID::kLeftHand)*0.75,XC->GetX(GenericHID::kLeftHand)*-0.75);
		Pivot->Set(IntakeJS->GetY()*.75);

		if (!limitTop->Get() && IntakeArm->GetY() < 0) {

			Elevator->ctre::phoenix::motorcontrol::can::WPI_TalonSRX::Set(0);
			Elevator2->Set(0);

		} else if (!limitBottom->Get() && IntakeArm->GetY() > 0) {

			Elevator->ctre::phoenix::motorcontrol::can::WPI_TalonSRX::Set(0);
			Elevator2->Set(0);

		} else {

			Elevator->ctre::phoenix::motorcontrol::can::WPI_TalonSRX::Set(IntakeArm->GetY());
			Elevator2->Set(IntakeArm->GetY());

		}

		if(IntakeJS->GetRawButton(1)){
			LeftIntake->Set(1);
			RightIntake->Set(-1);
		} else if(IntakeJS->GetRawButton(3)) {
			LeftIntake->Set(-0.5);
			RightIntake->Set(0.5);
		} else{
			LeftIntake->Set(0);
			RightIntake->Set(0);
		}
	}

	void TestPeriodic() {
		//lw->Run();
	}

private:
	Talon *Pivot;
	Talon *Elevator2;
	Spark *talonTL;
	Spark *talonBL;
	Spark *talonTR;
	Spark *talonBR;
	VictorSP *LeftIntake;
	VictorSP *RightIntake;
	ctre::phoenix::motorcontrol::can::WPI_TalonSRX *Elevator;
	Joystick *IntakeJS;
	Joystick *IntakeArm;
	Timer *timer;
	XboxController *XC;
	DifferentialDrive *drive;
	SpeedControllerGroup *leftGroup;
	SpeedControllerGroup *rightGroup;
	DigitalInput *limitTop;
	DigitalInput *limitBottom;
	ADIS16448_IMU *gyro;
	frc::LiveWindow* lw = LiveWindow::GetInstance();
	//frc::SendableChooser<std::string> chooser;
	//const std::string autoNameDefault = "Default";
	//const std::string autoNameCustom = "My Auto";
	//std::string autoSelected;
};


//class GyroSample : public SampleRobot
// {
//	  RobotDrive myRobot;
//	  AnalogGyro gyro;
//	  static const float kP = 0.03;
//
//	 GyroSample():
//		 myRobot(1,2),
//		 gyro(1)
//	  { myRobot.SetExpiration(0.1)
// }
START_ROBOT_CLASS(Robot)
