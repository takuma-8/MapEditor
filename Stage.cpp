#include "Stage.h"
#include "Engine/Model.h"
#include "Engine/Input.h"
#include "Engine/Camera.h"
#include "Engine/Fbx.h"
#include "resource.h"


void Stage::SetBlock(int _x, int _z, BLOCKTYPE _type)
{
	table_[_x][_z].type = _type;
}

void Stage::SetBlockHeight(int _x, int _z, int _height)
{
	table_[_x][_z].height = _height;
}

//コンストラクタ
Stage::Stage(GameObject* parent)
	:GameObject(parent, "Stage"), selectedAction(IDC_RADIO_UP), selectedTexture(0)
{
	for (int i = 0; i < MODEL_NUM; i++) {
		hModel_[i] = -1;
	}
	for (int z = 0; z < ZSIZE; z++) {
		for (int x = 0; x < XSIZE; x++) {
			SetBlock(x, z, DEFAULT);
		}
	}
}

//デストラクタ
Stage::~Stage()
{
}

//初期化
void Stage::Initialize()
{
	string modelname[] = {
		"BoxDefault.fbx",//デフォルト
		"BoxBrick.fbx",//レンガ
		"BoxGrass.fbx",//草
		"BoxSand.fbx",//砂
		"BoxWater.fbx"//水
	};
	string fname_base = "assets/";
	//モデルデータのロード
	for (int i = 0; i < MODEL_NUM; i++) {
		hModel_[i] = Model::Load(fname_base + modelname[i]);
		assert(hModel_[i] >= 0);
	}
	//tableにブロックのタイプをセットしてやろう！
	for (int z = 0; z < ZSIZE; z++) {
		for (int x = 0; x < XSIZE; x++) {
			SetBlock(x, z, (BLOCKTYPE)(0));
			SetBlockHeight(x, z, 0);
		}
	}

}

//更新
void Stage::Update()
{
	if (!Input::IsMouseButtonDown(0)) {
		return;
	}
	float w = (float)(Direct3D::scrWidth / 2.0f);
	float h = (float)(Direct3D::scrHeight / 2.0f);
	//Offsetx,y は0
	//minZ =0 maxZ = 1

	XMMATRIX vp =
	{
		 w,  0,  0, 0,
		 0, -h,  0, 0,
		 0,  0,  1, 0,
		 w,  h,  0, 1
	};

	//ビューポート
	XMMATRIX invVP = XMMatrixInverse(nullptr, vp);
	//プロジェクション変換
	XMMATRIX invProj = XMMatrixInverse(nullptr, Camera::GetProjectionMatrix());
	//ビュー変換
	XMMATRIX invView = XMMatrixInverse(nullptr, Camera::GetViewMatrix());
	XMFLOAT3 mousePosFront = Input::GetMousePosition();
	mousePosFront.z = 0.0f;
	XMFLOAT3 mousePosBack = Input::GetMousePosition();
	mousePosBack.z = 1.0f;
	//①　mousePosFrontをベクトルに変換
	XMVECTOR vMouseFront = XMLoadFloat3(&mousePosFront);
	//②　①にinvVP、invPrj、invViewをかける
	vMouseFront = XMVector3TransformCoord(vMouseFront, invVP * invProj * invView);
	//③　mousePosBackをベクトルに変換
	XMVECTOR vMouseBack = XMLoadFloat3(&mousePosBack);
	//④　③にinvVP、invPrj、invViewをかける
	vMouseBack = XMVector3TransformCoord(vMouseBack, invVP * invProj * invView);

	if (!Input::IsMouseButtonDown(0)) {
		return;
	}



	// ... 以前のマウス位置とレイキャストの処理

	for (int x = 0; x < 15; x++) {
		for (int z = 0; z < 15; z++) {
			for (int y = table_[x][z].height; y >= 0; y--) {
				RayCastData data;
				XMStoreFloat4(&data.start, vMouseFront);
				XMStoreFloat4(&data.dir, vMouseBack - vMouseFront);
				// レイキャストを行う前にモデルの位置を設定
				Transform trans;
				trans.position_.x = x;
				trans.position_.y = y;
				trans.position_.z = z;
				Model::SetTransform(hModel_[0], trans);
				// レイキャストを実行
				Model::RayCast(hModel_[0], data);

				// ラジオボタンの選択に応じたアクションを実行
				if (data.hit) {
					switch (selectedAction) {
					case IDC_RADIO_UP:
						// "上げる" アクション
						table_[x][z].height++;
						break;

					case IDC_RADIO_DOWN:
						// "下げる" アクション
						if (table_[x][z].height > 0) {
							table_[x][z].height--;
						}

						break;

					case IDC_RADIO_CHANGE:
						// "変更" アクション
						BLOCKTYPE newType = static_cast<BLOCKTYPE>(selectedTexture);
						table_[x][z].type = newType;
						table_[x][z].height++;
						break;


					}
				}
			}
		}
	}
}

//描画
void Stage::Draw()
{
	//Model::SetTransform(hModel_, transform_);
	//Model::Draw(hModel_);

	for (int x = 0; x < 15; x++)
	{
		for (int z = 0; z < 15; z++)
		{
			for (int y = 0; y < table_[x][z].height + 1; y++)
			{
				//table[x][z]からオブジェクトのタイプを取り出して書く！
				int type = table_[x][z].type;
				Transform trans;
				trans.position_.x = x;
				trans.position_.y = y;
				trans.position_.z = z;
				Model::SetTransform(hModel_[type], trans);
				Model::Draw(hModel_[type]);
			}
		}
	}
}

//開放
void Stage::Release()
{
}

BOOL Stage::DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	// selectedAction をクラスのメンバー変数として宣言
	static int selectedTexture = 0; // デフォルトテクスチャ

	switch (msg) {
	case WM_INITDIALOG:
		// ダイアログの初期化処理

		// ラジオボタンの初期状態を設定
		CheckRadioButton(hDlg, IDC_RADIO_UP, IDC_RADIO_CHANGE, IDC_RADIO_UP);
		SendMessage(GetDlgItem(hDlg, IDC_RADIO_UP), BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_RADIO_DOWN), BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_RADIO_CHANGE), BM_SETCHECK, BST_UNCHECKED, 0);

		//コンボボックスの初期値
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"デフォルト");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"レンガ");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"草原");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"砂地");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"水");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_SETCURSEL, 0, 0);
		return TRUE;

	case WM_COMMAND:
		// ボタンのクリックイベント処理
		switch (LOWORD(wParam)) {
		case IDC_RADIO_UP:
		case IDC_RADIO_DOWN:
		case IDC_RADIO_CHANGE:
			// ラジオボタンがクリックされたら、選択アクションを更新
			selectedAction = LOWORD(wParam);
			break;

			// 他のコントロールの処理
			// ...
		}

	case IDC_COMBO2:
		// コンボボックスの選択が変更された場合
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			// 現在のコンボボックスの選択項目を取得
			int selectedIndex = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);

			// selectedIndex に対応するテクスチャに基づいてブロックのテクスチャを変更
			if (selectedIndex >= 0 && selectedIndex < MODEL_NUM) {
				selectedTexture = selectedIndex;

				// ここでブロックのテクスチャを変更する
				for (int x = 0; x < 15; x++) {
					for (int z = 0; z < 15; z++) {
						table_[x][z].type = static_cast<BLOCKTYPE>(selectedTexture);
					}
				}
			}
		}
		break;

		return FALSE;
	}

	return FALSE;
}