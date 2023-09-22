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

//�R���X�g���N�^
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

//�f�X�g���N�^
Stage::~Stage()
{
}

//������
void Stage::Initialize()
{
	string modelname[] = {
		"BoxDefault.fbx",//�f�t�H���g
		"BoxBrick.fbx",//�����K
		"BoxGrass.fbx",//��
		"BoxSand.fbx",//��
		"BoxWater.fbx"//��
	};
	string fname_base = "assets/";
	//���f���f�[�^�̃��[�h
	for (int i = 0; i < MODEL_NUM; i++) {
		hModel_[i] = Model::Load(fname_base + modelname[i]);
		assert(hModel_[i] >= 0);
	}
	//table�Ƀu���b�N�̃^�C�v���Z�b�g���Ă�낤�I
	for (int z = 0; z < ZSIZE; z++) {
		for (int x = 0; x < XSIZE; x++) {
			SetBlock(x, z, (BLOCKTYPE)(0));
			SetBlockHeight(x, z, 0);
		}
	}

}

//�X�V
void Stage::Update()
{
	if (!Input::IsMouseButtonDown(0)) {
		return;
	}
	float w = (float)(Direct3D::scrWidth / 2.0f);
	float h = (float)(Direct3D::scrHeight / 2.0f);
	//Offsetx,y ��0
	//minZ =0 maxZ = 1

	XMMATRIX vp =
	{
		 w,  0,  0, 0,
		 0, -h,  0, 0,
		 0,  0,  1, 0,
		 w,  h,  0, 1
	};

	//�r���[�|�[�g
	XMMATRIX invVP = XMMatrixInverse(nullptr, vp);
	//�v���W�F�N�V�����ϊ�
	XMMATRIX invProj = XMMatrixInverse(nullptr, Camera::GetProjectionMatrix());
	//�r���[�ϊ�
	XMMATRIX invView = XMMatrixInverse(nullptr, Camera::GetViewMatrix());
	XMFLOAT3 mousePosFront = Input::GetMousePosition();
	mousePosFront.z = 0.0f;
	XMFLOAT3 mousePosBack = Input::GetMousePosition();
	mousePosBack.z = 1.0f;
	//�@�@mousePosFront���x�N�g���ɕϊ�
	XMVECTOR vMouseFront = XMLoadFloat3(&mousePosFront);
	//�A�@�@��invVP�AinvPrj�AinvView��������
	vMouseFront = XMVector3TransformCoord(vMouseFront, invVP * invProj * invView);
	//�B�@mousePosBack���x�N�g���ɕϊ�
	XMVECTOR vMouseBack = XMLoadFloat3(&mousePosBack);
	//�C�@�B��invVP�AinvPrj�AinvView��������
	vMouseBack = XMVector3TransformCoord(vMouseBack, invVP * invProj * invView);

	if (!Input::IsMouseButtonDown(0)) {
		return;
	}



	// ... �ȑO�̃}�E�X�ʒu�ƃ��C�L���X�g�̏���

	for (int x = 0; x < 15; x++) {
		for (int z = 0; z < 15; z++) {
			for (int y = table_[x][z].height; y >= 0; y--) {
				RayCastData data;
				XMStoreFloat4(&data.start, vMouseFront);
				XMStoreFloat4(&data.dir, vMouseBack - vMouseFront);
				// ���C�L���X�g���s���O�Ƀ��f���̈ʒu��ݒ�
				Transform trans;
				trans.position_.x = x;
				trans.position_.y = y;
				trans.position_.z = z;
				Model::SetTransform(hModel_[0], trans);
				// ���C�L���X�g�����s
				Model::RayCast(hModel_[0], data);

				// ���W�I�{�^���̑I���ɉ������A�N�V���������s
				if (data.hit) {
					switch (selectedAction) {
					case IDC_RADIO_UP:
						// "�グ��" �A�N�V����
						table_[x][z].height++;
						break;

					case IDC_RADIO_DOWN:
						// "������" �A�N�V����
						if (table_[x][z].height > 0) {
							table_[x][z].height--;
						}

						break;

					case IDC_RADIO_CHANGE:
						// "�ύX" �A�N�V����
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

//�`��
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
				//table[x][z]����I�u�W�F�N�g�̃^�C�v�����o���ď����I
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

//�J��
void Stage::Release()
{
}

BOOL Stage::DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	// selectedAction ���N���X�̃����o�[�ϐ��Ƃ��Đ錾
	static int selectedTexture = 0; // �f�t�H���g�e�N�X�`��

	switch (msg) {
	case WM_INITDIALOG:
		// �_�C�A���O�̏���������

		// ���W�I�{�^���̏�����Ԃ�ݒ�
		CheckRadioButton(hDlg, IDC_RADIO_UP, IDC_RADIO_CHANGE, IDC_RADIO_UP);
		SendMessage(GetDlgItem(hDlg, IDC_RADIO_UP), BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_RADIO_DOWN), BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_RADIO_CHANGE), BM_SETCHECK, BST_UNCHECKED, 0);

		//�R���{�{�b�N�X�̏����l
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"�f�t�H���g");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"�����K");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"����");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"���n");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)"��");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_SETCURSEL, 0, 0);
		return TRUE;

	case WM_COMMAND:
		// �{�^���̃N���b�N�C�x���g����
		switch (LOWORD(wParam)) {
		case IDC_RADIO_UP:
		case IDC_RADIO_DOWN:
		case IDC_RADIO_CHANGE:
			// ���W�I�{�^�����N���b�N���ꂽ��A�I���A�N�V�������X�V
			selectedAction = LOWORD(wParam);
			break;

			// ���̃R���g���[���̏���
			// ...
		}

	case IDC_COMBO2:
		// �R���{�{�b�N�X�̑I�����ύX���ꂽ�ꍇ
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			// ���݂̃R���{�{�b�N�X�̑I�����ڂ��擾
			int selectedIndex = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);

			// selectedIndex �ɑΉ�����e�N�X�`���Ɋ�Â��ău���b�N�̃e�N�X�`����ύX
			if (selectedIndex >= 0 && selectedIndex < MODEL_NUM) {
				selectedTexture = selectedIndex;

				// �����Ńu���b�N�̃e�N�X�`����ύX����
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