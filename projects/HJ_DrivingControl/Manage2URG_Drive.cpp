#include "DrivingControl.h"

// URGのパラメータをセットする
void Manage2URG_Drive::setURGParam(int URG_COM[], float URGPOS[][4], int NumOfURG)
{
	for (int i = 0; i < NumOfURG; i++)
	{
		urgdArray[i].init(URG_COM[i], URGPOS[i]);
	}
}
// デストラクタで配列の解放
Manage2URG_Drive::~Manage2URG_Drive()
{
}

// 左右のURGで障害物(仮)を検出した結果を総合して障害物の判断をする
urg_driving::ObstacleEmergency Manage2URG_Drive::checkObstacle()
{

	float* dataL[2], *dataR[2];
	float adis = 0.0, bdis = 0.0;
	int count[7] = {0, 0, 0, 0, 0, 0, 0};

	urgdArray[0].getObstacleData(dataR[0], dataR[1]);
	urgdArray[1].getObstacleData(dataL[0], dataL[1]);

	//cout << "点L：" << dataR[0][0] << endl;
	//cout << "点R：" << dataL[0][0] << endl;

	// ここに条件式1
	/*for (int i = 0; i < dataR[0][0]; i++)
	{
		for (int j = 0; j < dataL[0][0]; j++)
		{
			adis = pow((dataR[0][i] - dataL[0][j]), 2) + pow((dataR[1][i] - (dataL[1][j] + 280)), 2);
			if (adis < bdis)
			{
				bdis = adis;
			}
		}
		if (bdis < 2500){
			count += 1;
		}
	}*/


	//条件式2
	for (int i = 0; i < dataR[0][0]; i++){
		if (dataR[0][i] < 0.0 && dataR[1][i] < 0.0){
			count[3] += 1;
		}
		else if (dataR[0][i] > 0.0){
			for (int j = 0; j < dataL[0][0]; j++){
				if ((dataL[0][j] > 0)){
					adis = pow((dataR[0][i] - dataL[0][j]), 2) + pow((dataR[1][i] - (dataL[1][j] + 525.0)), 2);
					if (adis < bdis){
						bdis = adis;
					}
					if (bdis < 2500.0 && 600.0 < dataR[0][i]){
						count[1] += 1;
						break;
					}
					else if (bdis < 2500.0 && 500.0 < dataR[0][i] && dataR[0][i] < 600.0){
						count[2] += 1;
						break;
					}
					/*else if (bdis < 2500.0 && 0.0 < dataL[0][j] && dataL[1][j] < -650.0){
						count[5] += 1;
						break;
					}
					else if (bdis < 2500.0 && 0.0 < dataL[0][j] && 120.0 < dataL[1][j]){
						count[6] += 1;
						break;
					}*/
					else if (bdis < 2500.0 && dataR[0][i] < 500.0){
						count[0] += 1;
						break;
					}
				}
			}
		}
	}

	for (int k = 0; k < dataL[0][0]; k++){
		if (dataL[0][k] < 0.0 && dataL[1][k] > 0.0){
			count[4] += 1;
		}
	}

	for (int i = 0; i < 2; i++) delete[] dataL[i];
	for (int i = 0; i < 2; i++) delete[] dataR[i];

	if (count[0] > 20){
		//前方に障害物あり、停止する指令を送る
		return urg_driving::ObstacleEmergency::DETECT;
		//printf("点の数　= %d\n", count);
	}
	else if (count[3] > 10 || count[5] > 10){
		//左側に障害物あり、停止する指令を送る
		return urg_driving::ObstacleEmergency::DETECT_LEFT;
		//printf("点の数　= %d\n", count);
	}
	else if (count[4] > 10 || count[6] > 10){
		//右側に障害物あり、停止する指令を送る
		return urg_driving::ObstacleEmergency::DETECT_RIGHT;
		//printf("点の数　= %d\n", count);
	}
	else if (count[1] > 10){
		//速度を1段階下げる指令を送る
		return urg_driving::ObstacleEmergency::SLOW1;
	}
	else if (count[2] > 10){
		//速度を2段階下げる指令を送る
		return urg_driving::ObstacleEmergency::SLOW2;
	}

	return urg_driving::ObstacleEmergency::NONE;

}

// 停止した状態で周辺の画像を作成するやつ(TM用)
void Manage2URG_Drive::getAroundImage(int width, int height, int resolution, int measurementTimes)
{
	PCImage::isColor = false;
	urg_driving::initPCImage(width, height, resolution);
	urg_driving::setPCImageOrigin(width / 2, height / 2);

	PCImage::BGR color[2] = { PCImage::B, PCImage::G };
	for (int times = 0; times < measurementTimes; times++)
	{
		for (int i = 0; i < 1; i++)
		{
			urgdArray[i].setWriteLine(false);
			urgdArray[i].setPCImageColor(color[i]);
			urgdArray[i].writeMap(0, 0, 0);
		}
	}
	urg_driving::getPCImage(tmTemplate);
	//cv::imshow("show", tmTemplate);
}

void Manage2URG_Drive::readMapImage(string mapName)
{
	tmMap.push_back(cv::imread(mapName,0));
	if (false&&tmMap.back().empty())
	{
		cout << "False read Map image" << endl;
		int z = getchar();
	}
}