#include "DrivingControl.h"

const int ENCODER_COM = 10;
const int CONTROLLER_COM = 9;

void main()
{
	//URG��COM�|�[�g���w��
	int URG_COM[] = { 6, 27 };

	//URG�̈ʒu���w��
	float urgPOS[][4] = { 20.0, 350.0, -265.0, 0.5236,
		20.0, 350.0, 260.0, -0.5236 };

	DrivingFollowPath DFP("../../data/route/rouka_migi7.rt", 24.0086517664 / 1.005, 23.751783167, ENCODER_COM, CONTROLLER_COM);
	DFP.setURGParam(URG_COM, urgPOS, sizeof(URG_COM) / sizeof(int));
	DFP.readMapImage("../../data/route/roukaMap.jpg");
	DFP.run_FF();

	cout << "complete" << endl;
}
