//---------------------------------------------------------------------------

#include <vcl.h>
//===
#pragma warn -com
#include <C:\Program Files\Microsoft DirectX SDK (June 2010)\Include\D3D9.h>

#define __in
#define __in_bcount(arg)
#define __inout
#define __out_bcount(arg)
#define __out
#define __in_opt
#define __out_ecount_part_opt(arg1, arg2)
#define __in_ecount(arg)
#define __out_ecount(arg)
#define __out_bcount_opt(arg)
#define __in_bcount_opt(arg)
#define __in_ecount_opt(arg)
#define __out_ecount_opt(arg)
#define __out_opt
#define __inout_opt
#define sqrtf(arg) sqrt(arg)
#define WINAPI_INLINE inline WINAPI
#include <C:\Program Files\Microsoft DirectX SDK (June 2010)\Include\D3DX10Math.h>
#include <C:\Program Files\Microsoft DirectX SDK (June 2010)\Include\D3dx9tex.h>
#pragma warn +com

#include <vector>	// для std::vector
#include <map>		// для std::map
//===
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//===
//////////////////
// Общий раздел //
//////////////////
IDirect3D9* D3D;
bool VBoxMode;			// dev var
IDirect3DDevice9* D3DDevice;
bool IsRendering = false;
byte FPS = 0;
bool KeysMap[255], MouseButtonsMap[3];
bool StopSignal = false;
IDirect3DTexture9* Base_Texture;
std::vector<IDirect3DTexture9*> Textures;
const UINT PriorityBasis = 256, PB = PriorityBasis, PB_2 = PB * PB;
D3DXIMAGE_INFO TextureInfo;
float Aspect;
IDirect3DTexture9* LastLoadTexture;
IDirect3DTexture9* AddTexture(AnsiString FileName){
	if (D3DXCreateTextureFromFileEx(
		D3DDevice, ("Текстуры\\" + FileName).c_str(),
		0, 0, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
		0, &TextureInfo, NULL, &LastLoadTexture
	) == D3D_OK){
		Textures.push_back(LastLoadTexture);
	} else {
		LastLoadTexture = Base_Texture;
	}
	return LastLoadTexture;
}
int MouseX = 0, MouseY = 0;
const Screen_HalfW = Screen->Width / 2, Screen_HalfH = Screen->Height / 2;
float	X = 0, Y = 0, Z = 0,
	LastTime = Time().Val;
D3DMATERIAL9* Base_Material;
struct TexturePosition {
	float Left, Right, Top, Down;
	TexturePosition(float _Left, float _Right, float _Top, float _Down){
		Left = _Left; Right = _Right; Top = _Top; Down = _Down;
	}
};
TexturePosition Base_TexPos = TexturePosition(0, 1, 1, 0);

///////////////
// UI-раздел //
///////////////
	// UI-переменные
	float UI_kScreenPosition;
	D3DXMATRIX* UI_Base_ViewMatrix;
	IDirect3DVertexBuffer9* UI_Base_Quad;
	D3DXMATRIX UI_BaseMatrix;
	std::vector<IDirect3DVertexBuffer9*>	UI_Quads, UI_QuadsLinks;
	std::vector<IDirect3DTexture9*>		UI_Textures;
	std::vector<D3DXMATRIX*>		UI_Matrixes;
	std::vector<std::map<UINT, D3DXMATRIX*> > UI_TransformMatrixes;
	D3DXMATRIX UI_SharedMatrix; bool UI_use_SharedMatrix = false;	// "= false" поэтому не заполняется UI_SharedMatrix
	std::vector<bool> UI_PersistentFlag, UI_VisibleFlag;
	int UI_Cursor_FirstIndex, UI_Cursor_LastIndex;
	float UI_Cursor_ScaleCorrection;
	float UI_Cursor_animation = 0, UI_BackGround_animation = 0;

	// Функции UI-заполнения (и вспомогательные структуры)
	const DWORD UI_FVF = D3DFVF_XYZ | D3DFVF_TEX1;
	struct UI_Vertex {
		float x, y, z, u, v;
		/*UI_Vertex(float _x, float _y, float _z, float _u, float _v){
			x = _x; y = _y; z = _z; u = _u; v = _v;
		}*/
	};
	struct QuadGeometry {
		float XBegin, XShift, YBegin, YShift, Z;
		QuadGeometry(float _XBegin, float _XShift, float _YBegin, float _YShift, float _Z){
			XBegin = _XBegin; XShift = _XShift;
			YBegin = _YBegin; YShift = _YShift;
			Z = _Z;
		}
	};
	IDirect3DVertexBuffer9* UI_AddQuad(QuadGeometry Geometry, TexturePosition TexPos){
		IDirect3DVertexBuffer9* Quad;
		D3DDevice->CreateVertexBuffer(
			6 * sizeof(UI_Vertex),
			D3DUSAGE_WRITEONLY,
			UI_FVF,
			D3DPOOL_MANAGED,
			&Quad,
			NULL
		);
		UI_Vertex* V;
		Quad->Lock(0, 0, (void**)&V, 0);
		float	Left = Geometry.XBegin, Right = Geometry.XShift,
			Down = Geometry.YBegin, Top   = Geometry.YShift, Z = Geometry.Z;
		if (Right > 0){Right += Left;} else {float _Right = Left; Left += Right; Right = _Right;}
		if (Top   > 0){Top   += Down;} else {float _Top   = Down; Down += Top;   Top   = _Top;}
		float ToScreenPos = UI_kScreenPosition * (Z + 1.0);
		Left *= ToScreenPos; Right *= ToScreenPos; Top *= ToScreenPos; Down *= ToScreenPos;
		/*V[0] = UI_Vertex(Left,	Down,	Z,	TexPos.Left,	TexPos.Top);
		V[1] = UI_Vertex(Left,	Top,	Z,	TexPos.Left,	TexPos.Down);
		V[2] = UI_Vertex(Right,	Top,	Z,	TexPos.Right,	TexPos.Down);
		V[3] = UI_Vertex(Left,	Down,	Z,	TexPos.Left,	TexPos.Top);
		V[4] = UI_Vertex(Right,	Top,	Z,	TexPos.Right,	TexPos.Down);
		V[5] = UI_Vertex(Right,	Down,	Z,	TexPos.Right,	TexPos.Top);*/
		UI_Vertex QuadVertexes[] = {
			{Left,	Down,	Z,	TexPos.Left,	TexPos.Top},
			{Left,	Top,	Z,	TexPos.Left,	TexPos.Down},
			{Right,	Top,	Z,	TexPos.Right,	TexPos.Down},
			{Left,	Down,	Z,	TexPos.Left,	TexPos.Top},
			{Right,	Top,	Z,	TexPos.Right,	TexPos.Down},
			{Right,	Down,	Z,	TexPos.Right,	TexPos.Top}
		};
		memcpy(V, QuadVertexes, sizeof(QuadVertexes) );
		Quad->Unlock();
		UI_Quads.push_back(Quad);
		return Quad;
	}
	void UI_AddItem(IDirect3DVertexBuffer9* Quad, IDirect3DTexture9* Texture,
		std::map<UINT, D3DXMATRIX*> TransformMatrixes,
		D3DXMATRIX* Matrix, bool PersistentFlag, bool VisibleFlag
	){
		UI_QuadsLinks.push_back(Quad);
		UI_Textures.push_back(Texture);
		UI_TransformMatrixes.push_back(TransformMatrixes);
		UI_Matrixes.push_back(Matrix);
		UI_PersistentFlag.push_back(PersistentFlag);
		UI_VisibleFlag.push_back(VisibleFlag);
	}

///////////////
// 3D-раздел //
///////////////
	// Общий раздел
	const DWORD _3D_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	struct _3D_Vertex {
		float x, y, z, nx, ny, nz, u, v;
		/*_3D_Vertex(float _x, float _y, float _z, float _nx, float _ny, float _nz, float _u, float _v){
			x = _x; y = _y; z = _z; nx = _nx; ny = _ny; nz = _nz; u = _u; v = _v;
		}*/
	};
	struct _3D_VertexBufferGeometry{
		IDirect3DVertexBuffer9* VertexBuffer;
		IDirect3DTexture9* Texture;
		int PoligonsCount;
	};
	struct _3D_MeshGeometry{
		ID3DXMesh* Mesh;
		std::vector<D3DMATERIAL9> Materials;
		std::vector<IDirect3DTexture9*> Textures;
	};
	const _3D_GT_VertexBuffer = 0, _3D_GT_Mesh = 1;
	struct _3D_GeometryType{
		int	GeometryType;
		void*	GeometryData;
	};

	// Функция загрузки 3D-модели из Х-файла
	_3D_MeshGeometry* Base_MeshGeometry;
	_3D_MeshGeometry _3D_Load3DModelFromX(AnsiString FileName){
		ID3DXBuffer *adjBuffer, *mtrlBuffer;
		DWORD numMtrls;
		ID3DXMesh* Mesh;
		_3D_MeshGeometry MeshGeometry;
		if (FAILED(D3DXLoadMeshFromX(
			("Модели\\" + FileName).c_str(),
			D3DXMESH_MANAGED,
			D3DDevice,
			&adjBuffer,
			&mtrlBuffer,
			0,
			&numMtrls,
			&Mesh
		) ) ){
			MeshGeometry = *Base_MeshGeometry;
		} else {
			MeshGeometry.Mesh = Mesh;
			if (numMtrls){
				D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();
				std::map<AnsiString, IDirect3DTexture9*> LoadedTextures;
				for(DWORD i = 0; i < numMtrls; i++){
					// При загрузке в свойстве MatD3D не устанавливается
					// значение для фонового света, поэтому установим его
					// сейчас
					mtrls[i].MatD3D.Ambient = mtrls[i].MatD3D.Diffuse;

					MeshGeometry.Materials.push_back(mtrls[i].MatD3D);
					AnsiString TextureFilename = mtrls[i].pTextureFilename;
					IDirect3DTexture9* Texture;
					if (TextureFilename.Length() ){
						if (LoadedTextures.count(TextureFilename) == 0){
							Texture = AddTexture(TextureFilename);
							LoadedTextures[TextureFilename] = Texture;
						} else {
							Texture = LoadedTextures[TextureFilename];
						}
					} else {
						Texture = Base_Texture;
					}
					MeshGeometry.Textures.push_back(Texture);
				}
				LoadedTextures.clear();
				adjBuffer->Release();
				mtrlBuffer->Release();
			}
		}
		return MeshGeometry;
	}

	////////////////
	// CFP-раздел //
	////////////////
		// CFP -- Camera Fixed Points -- зафиксированные на камере точки
		// По-сути, оружие с руками.

		// Общий раздел
		std::vector<_3D_GeometryType> CFP_Geometry, CFP_GeometryLinks;
		std::vector<D3DXMATRIX> CFP_Matrixes;
		std::vector<std::map<UINT, D3DXMATRIX*> > CFP_TransformMatrixes;
		D3DXMATRIX CFP_SharedMatrix; bool CFP_use_SharedMatrix = false;		// "= false" поэтому не заполняется CFP_SharedMatrix
		std::vector<bool> CFP_PersistentFlag, CFP_VisibleFlag;

		// Функция добавления элемента в CFP
		// В идеале, вместо MeshGeometry нужно добавлять _3D_GeometryType,
		// и вынести добавление _3D_GeometryType в CFP_Geometry из функции.
		void CFP_AddItem(
			_3D_MeshGeometry* MeshGeometry,
			std::map<UINT, D3DXMATRIX*> TransformMatrixes, D3DXMATRIX Matrix,
			bool PersistentFlag, bool VisibleFlag, bool Add_to_MeshList
		){
			_3D_GeometryType Geometry = {_3D_GT_Mesh, (void*)MeshGeometry};
			if (MeshGeometry != Base_MeshGeometry && Add_to_MeshList){
				CFP_Geometry.push_back(Geometry);
			}
			CFP_GeometryLinks.push_back(Geometry);
			CFP_Matrixes.push_back(Matrix);
			CFP_TransformMatrixes.push_back(TransformMatrixes);
			CFP_PersistentFlag.push_back(PersistentFlag);
			CFP_VisibleFlag.push_back(VisibleFlag);
		}

		// CFP-перменные
		float Left_MashineGun_SpinSpeed = 0, Right_MashineGun_SpinSpeed = 0;

	////////////////
	// WFP-раздел //
	////////////////
		// WFP -- World Fixed Points -- зафиксированные в окружающем мире точки

		// Общий раздел
		std::vector<IDirect3DVertexBuffer9*> VertexBuffers;
		_3D_VertexBufferGeometry WFP_Base_Quad;
		std::vector<_3D_GeometryType> WFP_Geometry;
		struct WFP_Type {
			int	GeometryType;
			void*	GeometryData;
			D3DXMATRIX* Matrix;
			std::map<UINT, D3DXMATRIX*> TransformMatrixes;
			bool PersistentFlag, VisibleFlag;
		};
		int Sequence = 0;
		std::map<UINT, WFP_Type> WFP;
		/*std::vector<D3DXMATRIX*> FlyImg_SpinMatrixes;
		float TotalTime = 0;*/

		// Функция создания квада
		_3D_VertexBufferGeometry WFP_AddQuad(TexturePosition TexPos){
			IDirect3DVertexBuffer9* Quad;
			D3DDevice->CreateVertexBuffer(
				6 * sizeof(_3D_Vertex),
				D3DUSAGE_WRITEONLY,
				_3D_FVF,
				D3DPOOL_MANAGED,
				&Quad,
				NULL
			);
			_3D_Vertex* V;
			Quad->Lock(0, 0, (void**)&V, 0);
			_3D_Vertex QuadVertexes[] = {
				{-1, -1, 0, 0, 0, -1, TexPos.Left,  TexPos.Top},
				{-1,  1, 0, 0, 0, -1, TexPos.Left,  TexPos.Down},
				{ 1,  1, 0, 0, 0, -1, TexPos.Right, TexPos.Down},
				{-1, -1, 0, 0, 0, -1, TexPos.Left,  TexPos.Top},
				{ 1,  1, 0, 0, 0, -1, TexPos.Right, TexPos.Down},
				{ 1, -1, 0, 0, 0, -1, TexPos.Right, TexPos.Top}
			};
			memcpy(V, QuadVertexes, sizeof(QuadVertexes) );
			Quad->Unlock();
			VertexBuffers.push_back(Quad);
			_3D_VertexBufferGeometry QuadGeometry = {Quad, NULL, 2};;
			return QuadGeometry;
		}

		// Функция добавления геометрии
		_3D_GeometryType WFP_AddGeometry(_3D_MeshGeometry MeshGeometry){
			_3D_MeshGeometry* pMeshGeometry = new _3D_MeshGeometry;
			*pMeshGeometry = MeshGeometry;
			_3D_GeometryType Geometry = {_3D_GT_Mesh, pMeshGeometry};
			WFP_Geometry.push_back(Geometry);
			return Geometry;
		}
		_3D_GeometryType WFP_AddGeometry(
			_3D_VertexBufferGeometry VertexGeometry,
			IDirect3DTexture9* Texture
		){
			_3D_VertexBufferGeometry*
				pVertexGeometry = new _3D_VertexBufferGeometry;
			*pVertexGeometry = VertexGeometry;
			pVertexGeometry->Texture = Texture;
			_3D_GeometryType
				Geometry = {_3D_GT_VertexBuffer, pVertexGeometry};
			WFP_Geometry.push_back(Geometry);
			return Geometry;
		}

		// Функция добавления элемента в WFP (куска мира)
		void WFP_AddPiese(
			_3D_GeometryType Geometry,
			std::map<UINT, D3DXMATRIX*> TransformMatrixes, D3DXMATRIX Matrix,
			bool PersistentFlag, bool VisibleFlag
		){
			WFP_Type WTP_Piece;
			WTP_Piece.GeometryType = Geometry.GeometryType;
			WTP_Piece.GeometryData = Geometry.GeometryData;
			D3DXMATRIX* pMatrix = new D3DXMATRIX;
			*pMatrix = Matrix;
			WTP_Piece.Matrix = pMatrix;
			WTP_Piece.TransformMatrixes = TransformMatrixes;
			WTP_Piece.PersistentFlag = PersistentFlag;
			WTP_Piece.VisibleFlag = VisibleFlag;
			WFP[Sequence++] = WTP_Piece;
		}

		// Функция удаления элемента из WFP
		void WFP_DeletePiese(UINT PieseId){
			WFP_Type WFP_piese = WFP[PieseId];
			delete WFP_piese.Matrix;
			std::map<UINT, D3DXMATRIX*> TMs = WFP_piese.TransformMatrixes;
			typedef std::map<UINT, D3DXMATRIX*>::iterator MapIt;
			for (MapIt it2 = TMs.begin(); it2 != TMs.end(); ++it2){
				delete it2->second;
			}
			TMs.clear();
			WFP.erase(PieseId);
		}

		float	Shahta_Radius = 20, MadnessComp_Radius = 10;

//===
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner): TForm(Owner){
	////////////////////////
	// Инициализация окна //
	////////////////////////
		//WindowState	= wsMaximized;
		//BorderStyle	= bsNone;
		Cursor		= crNone;
		//Timer1->Name	= "Render";
		Render->Enabled	= false;
		Render->Interval= 1;
		Application->Title = "МС1";

	/////////////////////////////
	// Инициализация DirectX 9 //
	/////////////////////////////
		D3D = Direct3DCreate9(D3D_SDK_VERSION);
		if (!D3D){throw Exception("Ошибка: невозможно инициализировать DirectX!");}

		// Заполняем структуру D3DCAPS9 информацией о возможностях первичного видеоадаптера.
		D3DCAPS9 D3DCaps;
		D3D->GetDeviceCaps(
			D3DADAPTER_DEFAULT,	// Означает первичный видеоадаптер
			D3DDEVTYPE_HAL,		// Задает тип устройства, обычно D3DDEVTYPE_HAL
			&D3DCaps		/* Возвращает заполненную структуру D3DCAPS9,
							которая содержит информацию о
							возможностях видеоадаптера.
						*/
		);

		// Поддерживается аппаратная обработка вершин?
		DWORD vp;
		D3DDEVTYPE D3DDevType;		// Задает тип устройства, обычно D3DDEVTYPE_HAL
		if (D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT){
			// Да, сохраняем в vp флаг поддержки аппаратной обработки вершин.
			vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			D3DDevType = D3DDEVTYPE_HAL;
		} else {
			// Нет, сохраняем в vp флаг использования программной обработки вершин.
			vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			D3DDevType = D3DDEVTYPE_REF;
		}

		// Параметры показа
		D3DPRESENT_PARAMETERS D3DPP;
		D3DPP.BackBufferWidth	= ClientWidth;
		D3DPP.BackBufferHeight	= ClientHeight;
		D3DPP.BackBufferFormat	= D3DFMT_A8R8G8B8;	//формат пикселей
		D3DPP.BackBufferCount	= 1;
		D3DPP.MultiSampleType	= D3DMULTISAMPLE_8_SAMPLES;
		D3DPP.MultiSampleQuality= 0;
		D3DPP.SwapEffect	= D3DSWAPEFFECT_DISCARD;
		D3DPP.hDeviceWindow	= NULL;	//Handle;
		D3DPP.Windowed		= true;			// полноэкранный режим
		D3DPP.EnableAutoDepthStencil = true;
		D3DPP.AutoDepthStencilFormat = D3DFMT_D24S8;	// формат буфера

		// Глубины
		D3DPP.Flags = 0;
		D3DPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		D3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		// http://www.ishodniki.ru/art/art_progr/net/675.html
		// http://www.gamedev.ru/code/forum/?id=85737
		DWORD qualityLevels;
		/*if (SUCCEEDED(D3D->CheckDeviceMultiSampleType(			//SUCCEEDED важен без него проверка результата некоректна
			D3DADAPTER_DEFAULT, D3DDevType, D3DPP.BackBufferFormat, true,	//Ref = 1; Hal = 0
			D3DMULTISAMPLE_NONMASKABLE, &qualityLevels
		) ) / *&& SUCCEEDED(D3D->CheckDeviceMultiSampleType(
				D3DADAPTER_DEFAULT, D3DDevType, D3DPP.AutoDepthStencilFormat,
				true, D3DMULTISAMPLE_NONMASKABLE, &qualityLevels
		) )* / ){
			D3DPP.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
			D3DPP.MultiSampleQuality = qualityLevels - 1;
			//Text += ": MSAA Quality Level - " + D3DPP.MultiSampleQuality;
			ShowMessage(D3DPP.MultiSampleQuality);
		} else {
			D3DPP.MultiSampleType = D3DMULTISAMPLE_NONE;
			//Text += ": no MSAA";
			ShowMessage("D3DMULTISAMPLE_NONE");
		}*/

		D3DPP.MultiSampleType = D3DMULTISAMPLE_NONE;
		for (D3DMULTISAMPLE_TYPE mt = D3DMULTISAMPLE_16_SAMPLES; mt >= D3DMULTISAMPLE_2_SAMPLES; mt--){	//Ref = {8, 4}; Hal ={8 ,4, 2}
			if (SUCCEEDED(D3D->CheckDeviceMultiSampleType(
				D3DADAPTER_DEFAULT, D3DDevType, D3DPP.BackBufferFormat,
				true, mt, &qualityLevels
			) ) && SUCCEEDED(D3D->CheckDeviceMultiSampleType(
				D3DADAPTER_DEFAULT, D3DDevType, D3DPP.AutoDepthStencilFormat,
				true, mt, &qualityLevels
			) ) ){
				D3DPP.MultiSampleType = mt;
				D3DPP.MultiSampleQuality = qualityLevels - 1;
				//Text = String.Format("{0}: MSAA {1}x", Text, (int) mt);
				//ShowMessage(D3DPP.MultiSampleType);
				break;
			}
		}
		if (D3DPP.MultiSampleType == D3DMULTISAMPLE_NONE){
			//Text = String.Format("{0}: No MSAA", Text);
			//ShowMessage("D3DMULTISAMPLE_NONE");
		}

		// dev begin
			VBoxMode = (D3DDevType == D3DDEVTYPE_REF);
			if (VBoxMode){
				Cursor			= crDefault;
				D3DPP.BackBufferWidth	/= 12;
				D3DPP.BackBufferHeight	/= 12;
				D3DPP.MultiSampleType	=  D3DMULTISAMPLE_NONE;
			}
		// dev end

		if (FAILED(D3D->CreateDevice(
			D3DADAPTER_DEFAULT,	// первичный видеоадаптер
			D3DDevType,		// тип устройства
			Handle,			// окно, связанное с устройством
			vp,			// тип обработки вершин
			&D3DPP,			// параметры показа
			&D3DDevice		// возвращает созданное устройство
		) ) ){
			D3D->Release();
			throw Exception("Ошибка: невозможно создать устройство DirectX!");
		}

		D3DMATERIAL9 D3DMat;
		D3DCOLORVALUE	WHITE = {255, 255, 255, 255},
				BLACK = {  0,   0,   0, 255};
		D3DMat.Ambient	= WHITE;
		D3DMat.Diffuse	= WHITE;
		D3DMat.Specular	= WHITE;
		D3DMat.Emissive	= BLACK;
		D3DMat.Power	= 5.0f;
		//D3DDevice->SetMaterial(&D3DMat);

		Base_Material  = new D3DMATERIAL9;
		*Base_Material = D3DMat;

		D3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, true);

		// dev begin
			if (VBoxMode){
				D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
				D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);

				D3DDevice->SetRenderState(D3DRS_SPECULARENABLE,  true);
				D3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
				D3DDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 99);
			} else {
		// dev end

		D3DDevice->SetRenderState(D3DRS_SPECULARENABLE,  true);
		D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		D3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DSAMP_MAXANISOTROPY);
		D3DDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 99);

		// dev begin
			}
		// dev end

		// Задаем матрицу проекции
		D3DXMATRIX mProj;
		Aspect = (float)ClientWidth / (float)ClientHeight;
		D3DXMatrixPerspectiveFovLH(
			&mProj,
			D3DX_PI / 2.0,					// 90 градусов
			Aspect,
			0.001,
			1000000
		);
		D3DDevice->SetTransform(D3DTS_PROJECTION, &mProj);

		D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

	///////////////////
	// Заполнение UI //
	///////////////////
		// Инициализация общей части UI
		UI_kScreenPosition = tan( (D3DX_PI / 2.0) * 0.5);
		D3DDevice->SetRenderState(D3DRS_AMBIENT, D3DXCOLOR(1, 1, 1, 0) );
		UI_Base_ViewMatrix = new D3DXMATRIX;
		D3DXVECTOR3 position(0, 0, 0), target(0, 0, 1), up(0, 1, 0);
		D3DXMatrixLookAtLH(UI_Base_ViewMatrix, &position, &target, &up);
		UI_Base_Quad = UI_AddQuad(QuadGeometry(-1, 2, -1, 2, 0), Base_TexPos);
		D3DXMatrixTranslation(&UI_BaseMatrix, 0, 0, 1);
		Base_Texture = AddTexture("__Дефолтная текстурина.PNG");

		// Фон главного меню
		AnsiString BackgroundList[] = {
			"Фон главного меню.JPEG\\Объёмные кубики (oboi.tochka.net).jpg",
			"Фон главного меню.JPEG\\Обитаемая планета (goodoboi.ru).jpg",
			"Фон главного меню.JPEG\\brunettes-women-video-games-mirrors-edge-asians-wide.jpg",
			"Фон главного меню.JPEG\\serious_sam_yadro_smayl_pesok_piramida_1920x1080.jpg",
			"Фон главного меню.JPEG\\CAoxpmH1QZw.jpg"
		};
		randomize();
		IDirect3DTexture9* Texture = AddTexture(BackgroundList[rand() % 5]);
		std::map<UINT, D3DXMATRIX*> TransformMatrixes;
		D3DXMATRIX* Matrix = new D3DXMATRIX;
		float	TextureAspect = float(TextureInfo.Width) / float(TextureInfo.Height),
			ScaleX = TextureAspect, ScaleY = 1;
		if (Aspect > TextureAspect){ScaleX = Aspect; ScaleY = Aspect / TextureAspect;}
		D3DXMatrixScaling(Matrix, 1.2 * ScaleX, 1.2 * ScaleY, 1);
		TransformMatrixes[PB_2 * 1] = Matrix;
		Matrix = new D3DXMATRIX;
		TransformMatrixes[PB_2 * 1 + PB * 1] = Matrix;
		Matrix = new D3DXMATRIX;
		TransformMatrixes[PB_2 * 1 + PB * 2] = Matrix;
		Matrix = new D3DXMATRIX;
		UI_AddItem(UI_Base_Quad, Texture, TransformMatrixes, Matrix, false, true);

		// Тестовая активная зона
		//Texture = AddTexture("__Дефолтная текстурина.PNG");
		/*IDirect3DVertexBuffer9* UI_TestActiveZone_Quad = UI_AddQuad(
			QuadGeometry(-1, 2, -1, 2, 1), (TexturePosition(
				0, 0.5 * float(TextureInfo.Width) * Aspect, 0.5 * float(TextureInfo.Height), 0
			) )
		);*/
		TransformMatrixes.clear();
		Matrix = new D3DXMATRIX;
		D3DXMatrixScaling(Matrix, 0.1 * Aspect * (1.0 - 0.2), 0.1 * (1.0 - 0.2), 1);
		D3DXMATRIX MatrixX;
		D3DXMatrixTranslation(&MatrixX, 0, 0, -0.2);
		*Matrix *= MatrixX;
		TransformMatrixes[PB_2 * 1] = Matrix;
		Matrix = new D3DXMATRIX;
		TransformMatrixes[PB_2 * 1 + PB * 1] = Matrix;
		Matrix = new D3DXMATRIX;
		UI_AddItem(UI_Base_Quad, Texture, TransformMatrixes, Matrix, false, true);
		TransformMatrixes.clear();
		Matrix = new D3DXMATRIX;
		D3DXMatrixScaling(Matrix, 0.01 * Aspect, 0.01, 1);
		UI_AddItem(UI_Base_Quad, Texture, TransformMatrixes, Matrix, true, true);

		// Курсор и прицел
		UI_Cursor_FirstIndex = UI_TransformMatrixes.size();
		Texture = AddTexture("Цвет заливки курсора и прицела.PNG");
		// CC = Cursor_and_Crossbow = курсор с прицелом
		D3DXMATRIX	*UI_CC_ScaleMatrix = new D3DXMATRIX,	// Размеры CC
				*UI_CC_PaddingMatrix = new D3DXMATRIX;	// Отступ центра элемента CC от центра CC
		float ZShift = -0.3;
		UI_Cursor_ScaleCorrection = (1.0 + ZShift);
		D3DXMatrixScaling(UI_CC_ScaleMatrix,
			0.019 * UI_Cursor_ScaleCorrection,
			0.007 * UI_Cursor_ScaleCorrection,
			1
		);
		D3DXMatrixTranslation(UI_CC_PaddingMatrix,
			0, 0.037 * UI_kScreenPosition * 1.0 * UI_Cursor_ScaleCorrection, ZShift
		);
		// 4 радиальных(?) элемента курсора (длина больше высоты)
		for (float i = 0; i < 4; i++){
			TransformMatrixes.clear();
			TransformMatrixes[PB_2 * 1] = UI_CC_ScaleMatrix;
			Matrix = new D3DXMATRIX;
			TransformMatrixes[PB_2 * 1 + PB * 1] = Matrix;
			TransformMatrixes[PB_2 * 2] = UI_CC_PaddingMatrix;
			Matrix = new D3DXMATRIX;
			TransformMatrixes[PB_2 * 2 + PB * 1] = Matrix;
			Matrix = new D3DXMATRIX;
			D3DXMatrixRotationZ(Matrix, float(i) * D3DX_PI / 2.0);
			TransformMatrixes[PB_2 * 3] = Matrix;
			Matrix = new D3DXMATRIX;
			TransformMatrixes[PB_2 * 3 + PB * 1] = Matrix;
			Matrix = new D3DXMATRIX;
			UI_AddItem(UI_Base_Quad, Texture, TransformMatrixes, Matrix, false, true);
		}
		UI_CC_ScaleMatrix = new D3DXMATRIX;
		D3DXMatrixScaling(UI_CC_ScaleMatrix,
			0.007 * UI_Cursor_ScaleCorrection,
			0.015 * UI_Cursor_ScaleCorrection,
			1
		);
		UI_CC_PaddingMatrix = new D3DXMATRIX;
		D3DXMatrixTranslation(UI_CC_PaddingMatrix,
			0, 0.03 * UI_kScreenPosition * 1.0 * UI_Cursor_ScaleCorrection, ZShift
		);
		// 4 фронтальных(?) элемента курсора (длина меньше высоты)
		for (int i = 0; i < 4; i++){
			TransformMatrixes.clear();
			TransformMatrixes[PB_2 * 1] = UI_CC_ScaleMatrix;
			Matrix = new D3DXMATRIX;
			TransformMatrixes[PB_2 * 1 + PB * 1] = Matrix;
			TransformMatrixes[PB_2 * 2] = UI_CC_PaddingMatrix;
			Matrix = new D3DXMATRIX;
			TransformMatrixes[PB_2 * 2 + PB * 1] = Matrix;
			Matrix = new D3DXMATRIX;
			D3DXMatrixRotationZ(Matrix, (float(i) + 0.5) * D3DX_PI / 2.0);
			TransformMatrixes[PB_2 * 3] = Matrix;
			Matrix = new D3DXMATRIX;
			TransformMatrixes[PB_2 * 3 + PB * 1] = Matrix;
			Matrix = new D3DXMATRIX;
			UI_AddItem(UI_Base_Quad, Texture, TransformMatrixes, Matrix, false, true);
		}
		UI_Cursor_LastIndex = UI_TransformMatrixes.size();

	//////////////////////////
	// Добавление пулемётов //
	//////////////////////////
		IDirect3DTexture9 *BarrelTexture1 = AddTexture("Стволы {1}.PNG"),
				  *BarrelTexture2 = AddTexture("Стволы {2}.PNG"),
				  *BarrelTexture3 = AddTexture("Стволы {3}.PNG");
		UINT Slices = 31 /*- 1*/;
		if (VBoxMode){Slices = 13 /*- 1*/;}	// dev
		ID3DXMesh* Cylinder;
		D3DXCreateCylinder(D3DDevice, 1, 1, 1, Slices, 1, &Cylinder, NULL);
		_3D_MeshGeometry* MeshGeometry = new _3D_MeshGeometry;
		MeshGeometry->Mesh = Cylinder;
		_3D_GeometryType GeometryType;
		GeometryType.GeometryType = _3D_GT_Mesh;
		GeometryType.GeometryData = MeshGeometry;
		CFP_Geometry.push_back(GeometryType);
		struct MachinegunParameters {
			IDirect3DTexture9 *BarrelTexture1, *BarrelTexture2, *BarrelTexture3;
			ID3DXMesh* Mesh;
		} MachinPar = {BarrelTexture1, BarrelTexture2, BarrelTexture3, Cylinder};
		class Machinegun{
			public:
			static void Add(D3DXVECTOR3 Position, MachinegunParameters Params){
				// Параметры пулемёта
				int	BarrelsCount	= 6;	// Кол-во стволов пулемёта
				float	BarrelsRadius	= 0.02,
					BarrelsLength	= 1,
					MahinegunRadius	= 0.1;

				// Добавление стволов
				_3D_MeshGeometry* MeshGeometry;
				std::map<UINT, D3DXMATRIX*> TransformMatrixes;
				D3DXMATRIX *Barrels_BaseMatrix = new D3DXMATRIX,
					   *Matrix = new D3DXMATRIX;
				D3DXMatrixScaling(
					Barrels_BaseMatrix,
					BarrelsRadius, BarrelsRadius, BarrelsLength
				);
				D3DXMatrixTranslation(
					Matrix,
					0, 0.7 * MahinegunRadius, BarrelsLength / 2.0
				);
				*Barrels_BaseMatrix *= *Matrix;
				float AngleStep = D3DX_PI * 2.0 / float(BarrelsCount);
				for (int i = 0; i < BarrelsCount; i++){
					MeshGeometry = new _3D_MeshGeometry;
					MeshGeometry->Mesh = Params.Mesh;
					MeshGeometry->Materials.push_back(*Base_Material);
					MeshGeometry->Textures.push_back(Params.BarrelTexture1);
					TransformMatrixes.clear();
					TransformMatrixes[PB_2 * 1] = Barrels_BaseMatrix;
					Matrix = new D3DXMATRIX;
					D3DXMatrixRotationZ(Matrix, float(i) * AngleStep);
					TransformMatrixes[PB_2 * 1 + PB * 1] = Matrix;
					Matrix = new D3DXMATRIX;
					D3DXMatrixRotationZ(Matrix, 0);
					TransformMatrixes[PB_2 * 1 + PB * 2] = Matrix;
					Matrix = new D3DXMATRIX;
					D3DXMatrixTranslation(
						Matrix,
						Position.x, Position.y, Position.z
					);
					TransformMatrixes[PB_2 * 1 + PB * 3] = Matrix;
					Matrix = new D3DXMATRIX;
					D3DXMatrixRotationZ(Matrix, 0);
					CFP_AddItem(MeshGeometry, TransformMatrixes, *Matrix, false, true, false);
				}

				// Добавление передней перемычки
				MeshGeometry = new _3D_MeshGeometry;
				MeshGeometry->Mesh = Params.Mesh;
				MeshGeometry->Materials.push_back(*Base_Material);
				MeshGeometry->Textures.push_back(Params.BarrelTexture2);
				TransformMatrixes.clear();
				D3DXMATRIX* Barrels_BaseMatrix2 = new D3DXMATRIX;
				D3DXMatrixScaling(
					Barrels_BaseMatrix2,
					MahinegunRadius, MahinegunRadius, 0.05
				);
				Matrix = new D3DXMATRIX;
				D3DXMatrixTranslation(Matrix, 0, 0, 0.85 * BarrelsLength);
				*Barrels_BaseMatrix2 *= *Matrix;
				TransformMatrixes[PB_2 * 1] = Barrels_BaseMatrix2;
				D3DXMatrixRotationZ(Matrix, 0);
				TransformMatrixes[PB_2 * 1 + PB * 2] = Matrix;
				Matrix = new D3DXMATRIX;
				D3DXMatrixTranslation(
					Matrix,
					Position.x, Position.y, Position.z
				);
				TransformMatrixes[PB_2 * 1 + PB * 3] = Matrix;
				Matrix = new D3DXMATRIX;
				D3DXMatrixRotationZ(Matrix, 0);
				CFP_AddItem(MeshGeometry, TransformMatrixes, *Matrix, false, true, false);

				// Добавление задней перемычки
				MeshGeometry = new _3D_MeshGeometry;
				MeshGeometry->Mesh = Params.Mesh;
				MeshGeometry->Materials.push_back(*Base_Material);
				MeshGeometry->Textures.push_back(Params.BarrelTexture2);
				TransformMatrixes.clear();
				D3DXMATRIX* Barrels_BaseMatrix3 = new D3DXMATRIX;
				D3DXMatrixScaling(
					Barrels_BaseMatrix3,
					MahinegunRadius, MahinegunRadius, 1
				);
				Matrix = new D3DXMATRIX;
				D3DXMatrixTranslation(Matrix, 0, 0, -0.15 * BarrelsLength);
				*Barrels_BaseMatrix3 *= *Matrix;
				TransformMatrixes[PB_2 * 1] = Barrels_BaseMatrix3;
				D3DXMatrixRotationZ(Matrix, 0);
				TransformMatrixes[PB_2 * 1 + PB * 2] = Matrix;
				Matrix = new D3DXMATRIX;
				D3DXMatrixTranslation(
					Matrix,
					Position.x, Position.y, Position.z
				);
				TransformMatrixes[PB_2 * 1 + PB * 3] = Matrix;
				Matrix = new D3DXMATRIX;
				D3DXMatrixRotationZ(Matrix, 0);
				CFP_AddItem(MeshGeometry, TransformMatrixes, *Matrix, false, true, false);
			}
		};
		Machinegun::Add(D3DXVECTOR3(-0.3, -0.3, 0), MachinPar);
		Machinegun::Add(D3DXVECTOR3( 0.3, -0.3, 0), MachinPar);

	/////////////////////
	// Заполнение мира //
	/////////////////////
		WFP_Base_Quad = WFP_AddQuad(Base_TexPos);

		float Shahta_Height = 50/*, Shahta_Radius = 20, MadnessComp_Radius = 10*/;
		UINT Shahta_Slices = 31, MadnessComp_Slices = 31;

		TexturePosition
			Shahta_TexPos_TopDown	 = TexturePosition(0, 1, 1, 0),
			Shahta_TexPos_Walls	 = TexturePosition(0, 1, 10, 0),
			MadnessComp_TexPos_Walls = TexturePosition(0, 1, 1, 0);
		_3D_VertexBufferGeometry
			Shahta_TopDown_Quad    = WFP_AddQuad(Shahta_TexPos_TopDown),
			Shahta_Walls_Quad      = WFP_AddQuad(Shahta_TexPos_Walls),
			MadnessComp_Walls_Quad = WFP_AddQuad(MadnessComp_TexPos_Walls);
		IDirect3DTexture9
			*Shahta_TopDown_Texture    = AddTexture("Стены шахты.JPEG"),
			*Shahta_Walls_Texture      = AddTexture("Стены шахты.JPEG"),
			*MadnessComp_Walls_Texture = AddTexture("Стены шахты.JPEG");
		_3D_GeometryType
			Shahta_TopDown_Geometry    = WFP_AddGeometry(Shahta_TopDown_Quad,    Shahta_TopDown_Texture),
			Shahta_Walls_Geometry      = WFP_AddGeometry(Shahta_Walls_Quad,      Shahta_Walls_Texture),
			MadnessComp_Walls_Geometry = WFP_AddGeometry(MadnessComp_Walls_Quad, MadnessComp_Walls_Texture);

		D3DXMATRIX mMatrix, *pMatrix = new D3DXMATRIX,
			   Shahta_TopDown_BaseMatrix;
		D3DXMatrixScaling(&Shahta_TopDown_BaseMatrix, 2.0 * Shahta_Radius, 2.0 * Shahta_Radius, 0);

		// Пол
		TransformMatrixes.clear();
		mMatrix = Shahta_TopDown_BaseMatrix;
		mMatrix *= *D3DXMatrixRotationX(pMatrix, D3DX_PI / 2.0);
		WFP_AddPiese(
			Shahta_TopDown_Geometry,
			TransformMatrixes, mMatrix,
			true, true
		);

		// Потолок
		TransformMatrixes.clear();
		mMatrix = Shahta_TopDown_BaseMatrix;
		mMatrix *= *D3DXMatrixRotationX(pMatrix, -D3DX_PI / 2.0);
		mMatrix *= *D3DXMatrixTranslation(pMatrix, 0, Shahta_Height, 0);
		WFP_AddPiese(
			Shahta_TopDown_Geometry,
			TransformMatrixes, mMatrix,
			true, true
		);

		// Стены
		D3DXMATRIX Shahta_Walls_Base_Scale, Shahta_Walls_Base_YZ;
		D3DXMatrixScaling(
			&Shahta_Walls_Base_Scale,
			Shahta_Radius * tan(D3DX_PI / float(Shahta_Slices) ),
			Shahta_Height / 2.0, 1
		);
		D3DXMatrixTranslation(
			&Shahta_Walls_Base_YZ,
			0, Shahta_Height / 2.0, Shahta_Radius
		);
		float Rad_on_Wall = 2.0 * D3DX_PI / float(Shahta_Slices);
		for (UINT i = 0; i < Shahta_Slices; i++){
			TransformMatrixes.clear();
			mMatrix = Shahta_Walls_Base_Scale * Shahta_Walls_Base_YZ *
				*D3DXMatrixRotationY(pMatrix, float(i) * Rad_on_Wall);
			WFP_AddPiese(
				Shahta_Walls_Geometry,
				TransformMatrixes, mMatrix,
				true, true
			);
		}

		// Стены безумной машины
		D3DXMATRIX MadnessComp_Walls_Base_Scale, MadnessComp_Walls_Base_YZ;
		D3DXMatrixScaling(
			&MadnessComp_Walls_Base_Scale,
			MadnessComp_Radius * tan(D3DX_PI / float(MadnessComp_Slices) ),
			Shahta_Height / 2.0, 1
		);
		D3DXMatrixTranslation(
			&MadnessComp_Walls_Base_YZ,
			0, Shahta_Height / 2.0, -MadnessComp_Radius
		);
		Rad_on_Wall = 2.0 * D3DX_PI / float(MadnessComp_Slices);
		for (UINT i = 0; i < MadnessComp_Slices; i++){
			TransformMatrixes.clear();
			mMatrix = MadnessComp_Walls_Base_Scale * MadnessComp_Walls_Base_YZ *
				*D3DXMatrixRotationY(pMatrix, float(i) * Rad_on_Wall);
			WFP_AddPiese(
				MadnessComp_Walls_Geometry,
				TransformMatrixes, mMatrix,
				true, true
			);
		}

		Y = 2;

	///////////////
	// Остальное //
	///////////////
		ZeroMemory(KeysMap, sizeof(KeysMap) );
		ZeroMemory(MouseButtonsMap, sizeof(MouseButtonsMap) );
		SetCursorPos(Screen_HalfW, Screen_HalfH);
		Render->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RenderTimer(TObject*){
	if (IsRendering){return;}
	IsRendering = true;

	if (StopSignal){
		// Удаление UI
		for (size_t i = 0; i < UI_Quads.size(); i++){UI_Quads[i]->Release();}
		UI_Quads.clear();
		UI_QuadsLinks.clear();
		UI_Textures.clear();
		for (size_t i = 0; i < UI_Matrixes.size(); i++){delete UI_Matrixes[i];}
		UI_Matrixes.clear();
		for (size_t i = 0; i < UI_TransformMatrixes.size(); i++){
			std::map<UINT, D3DXMATRIX*> TransformMatrix = UI_TransformMatrixes[i];
			typedef std::map<UINT, D3DXMATRIX*>::iterator MapIt;
			for (MapIt it = TransformMatrix.begin(); it != TransformMatrix.end(); ++it){
				delete it->second;
			}
			TransformMatrix.clear();
		}
		UI_TransformMatrixes.clear();
		UI_PersistentFlag.clear();
		UI_VisibleFlag.clear();
		delete UI_Base_ViewMatrix;

		// Удаление CFP (оружия с руками)
		for (size_t i = 0; i < CFP_Geometry.size(); i++){
			_3D_GeometryType Geometry = CFP_Geometry[i];
			if (Geometry.GeometryType == _3D_GT_VertexBuffer){
				_3D_VertexBufferGeometry* VBG = (_3D_VertexBufferGeometry*)Geometry.GeometryData;
				VBG->VertexBuffer->Release();
			} else /*if (CFP_Geometry.GeometryType == _3D_GT_Mesh)*/{
				_3D_MeshGeometry* MeshData = (_3D_MeshGeometry*)Geometry.GeometryData;
				MeshData->Mesh->Release();
				MeshData->Materials.clear();
				MeshData->Textures.clear();
			}
			delete Geometry.GeometryData;
		}
		CFP_Geometry.clear();
		CFP_GeometryLinks.clear();
		CFP_Matrixes.clear();
		for (size_t i = 0; i < CFP_TransformMatrixes.size(); i++){
			std::map<UINT, D3DXMATRIX*> TransformMatrix = CFP_TransformMatrixes[i];
			typedef std::map<UINT, D3DXMATRIX*>::iterator MapIt;
			for (MapIt it = TransformMatrix.begin(); it != TransformMatrix.end(); ++it){
				delete it->second;
			}
			TransformMatrix.clear();
		}
		CFP_TransformMatrixes.clear();
		CFP_PersistentFlag.clear();
		CFP_VisibleFlag.clear();
		// Удалить меш ствола пулемута.

		// Удаление мира
		for (size_t i = 0; i < VertexBuffers.size(); i++){
			VertexBuffers[i]->Release();
		}
		VertexBuffers.clear();
		for (size_t i = 0; i < WFP_Geometry.size(); i++){
			_3D_GeometryType Geometry = WFP_Geometry[i];
			if (Geometry.GeometryType == _3D_GT_Mesh){
				_3D_MeshGeometry* MeshData = (_3D_MeshGeometry*)Geometry.GeometryData;
				MeshData->Mesh->Release();
				MeshData->Materials.clear();
				MeshData->Textures.clear();
			}
			delete Geometry.GeometryData;
		}
		WFP_Geometry.clear();
		for (std::map<UINT, WFP_Type>::iterator it = WFP.begin(); it != WFP.end(); ++it){
			WFP_Type WFP_piese = it->second;
			delete WFP_piese.Matrix;
			std::map<UINT, D3DXMATRIX*> TMs = WFP_piese.TransformMatrixes;
			typedef std::map<UINT, D3DXMATRIX*>::iterator MapIt;
			for (MapIt it2 = TMs.begin(); it2 != TMs.end(); ++it2){
				delete it2->second;
			}
			TMs.clear();
		}
		WFP.clear();

		// Удаление текстур и общих динамически созданных ресурсов
		for (size_t i = 0; i < Textures.size(); i++){Textures[i]->Release();}
		delete Base_Material;
		Textures.clear();
		D3DDevice->Release();
		D3D->Release();

		// Закрытие проги
		Close();
		return;
	}

	// Время с момента рендеринга последнего кадра
	float CurrentTime = Time().Val, DeltaTime = CurrentTime - LastTime;
	LastTime = CurrentTime;

	// Перемещение курсора
	// Проблема 1. Нужно ли полагаться на автопересчёт? Или лучше напрямую просчитать матрицу элементов курсора?
	POINT CursorPos;
	GetCursorPos(&CursorPos);
	// dev begin
		if (VBoxMode){
			MouseX = CursorPos.x - Screen_HalfW;
			MouseY = Screen_HalfH - CursorPos.y;
		} else {
	// dev end
	MouseX += CursorPos.x - Screen_HalfW;
	MouseY += Screen_HalfH - CursorPos.y;
	SetCursorPos(Screen_HalfW, Screen_HalfH);
	// dev begin
		}
	// dev end
	/*if (MouseX < -Screen_HalfW){
		MouseX = -Screen_HalfW;
	} else if (MouseX >= Screen_HalfW){
		MouseX = Screen_HalfW;
	}
	if (MouseY < -Screen_HalfH){
		MouseY = -Screen_HalfH;
	} else if (MouseY >= Screen_HalfH){
		MouseY = Screen_HalfH;
	}*/
	float	kCursor = UI_kScreenPosition * UI_Cursor_ScaleCorrection,
		Cursor_X = kCursor * Aspect * float(MouseX) / float(Screen_HalfW),
		Cursor_Y = kCursor * 1.0    * float(MouseY) / float(Screen_HalfH);
	D3DXMATRIX mCursorPos;
	D3DXMatrixTranslation(&mCursorPos, Cursor_X, Cursor_Y, 0);
	for (int i = UI_Cursor_FirstIndex; i < UI_Cursor_LastIndex; i++){
		*(UI_TransformMatrixes[i][PB_2 * 3 + PB * 1]) = mCursorPos;
	}

	///////////////// Анимация курсора????
	Cursor_X /= UI_kScreenPosition * UI_Cursor_ScaleCorrection;
	Cursor_Y /= UI_kScreenPosition * UI_Cursor_ScaleCorrection;
	if (Cursor_X > (-Aspect * 0.1) && Cursor_X < (Aspect * 0.1) && Cursor_Y > -0.1 && Cursor_Y < 0.1){
		UI_Cursor_animation -= 0.1;
		if (UI_Cursor_animation < 0.0){UI_Cursor_animation = 0.0;}
	} else {
		UI_Cursor_animation += 0.1;
		if (UI_Cursor_animation > 1.0){UI_Cursor_animation = 1.0;}
	}
	int	UI_CC_IndexOf_ScaleMatrix   = PB_2 * 1 + PB * 1,
		UI_CC_IndexOf_PaddingMatrix = PB_2 * 2 + PB * 1;
	D3DXMATRIX UI_CC_ScaleMatrix, UI_CC_PaddingMatrix;
	D3DXMatrixScaling(&UI_CC_ScaleMatrix,
		1.0 + UI_Cursor_animation * (2.5 - 1.0),
		1.0 + UI_Cursor_animation * (0.5 - 1.0),
		1
	);
	D3DXMatrixTranslation(&UI_CC_PaddingMatrix,
		0,
		0.0 + UI_Cursor_animation * (0.015 * UI_kScreenPosition * 1.0 - 0.0),
		0
	);
	for (int i = UI_Cursor_FirstIndex; i < UI_Cursor_LastIndex; i++){
		*(UI_TransformMatrixes[i][UI_CC_IndexOf_ScaleMatrix  ]) = UI_CC_ScaleMatrix;
		*(UI_TransformMatrixes[i][UI_CC_IndexOf_PaddingMatrix]) = UI_CC_PaddingMatrix;
	}

	// Вращательная анимация фона главного меню
	int UI_Background_Index = 0;
	D3DXMATRIX Background_XRotation, Background_YRotation;
	D3DXMatrixRotationX(&Background_XRotation, -0.01 * D3DX_PI * Cursor_Y);
	D3DXMatrixRotationY(&Background_YRotation,  0.01 * D3DX_PI * Cursor_X / Aspect);
	*(UI_TransformMatrixes[UI_Background_Index][PB_2 * 1 + PB * 1]) = Background_XRotation * Background_YRotation;

	// Анимацмя биения фона главного меню
	UI_BackGround_animation += 0.0008;
	D3DXMATRIX Background_ZShift;
	float BackGround_dZ = sin(D3DX_PI * 2.0 * UI_BackGround_animation - D3DX_PI / 2.0);
	BackGround_dZ = (1.0 + BackGround_dZ) / 2.0;
	D3DXMatrixTranslation(&Background_ZShift, 0, 0, 0.05 * BackGround_dZ);
	*(UI_TransformMatrixes[UI_Background_Index][PB_2 * 1 + PB * 2]) = Background_ZShift;

	// ////Вращение главного меню?
	int UI_MainMenu_Index = 1;
	*(UI_TransformMatrixes[UI_MainMenu_Index][PB_2 * 1 + PB * 1]) = Background_XRotation * Background_YRotation;

	// Перерасчёт конечных матриц элементов UI
	// Проблема 1. Нужен ли автопересчёт? Или лучше напрямую в каждом скрипте?
	for (size_t i = 0; i < UI_TransformMatrixes.size(); i++){
		if (!UI_PersistentFlag[i] && UI_VisibleFlag[i]){
			std::map<UINT, D3DXMATRIX*> TransformMatrix = UI_TransformMatrixes[i];
			std::map<UINT, D3DXMATRIX*>::iterator it = TransformMatrix.begin();
			D3DXMATRIX UI_Matrix = *it->second;
			for (it = ++it; it != TransformMatrix.end(); ++it){
				UI_Matrix *= *it->second;
			}
			*UI_Matrixes[i] = UI_Matrix;
		}
	}

	// Перерасчёт конечных матриц элементов CFP (Camera Fixed Points)
	for (size_t i = 0; i < CFP_TransformMatrixes.size(); i++){
		if (!CFP_PersistentFlag[i] && CFP_VisibleFlag[i]){
			std::map<UINT, D3DXMATRIX*> TransformMatrix = CFP_TransformMatrixes[i];
			std::map<UINT, D3DXMATRIX*>::iterator it = TransformMatrix.begin();
			D3DXMATRIX CFP_Matrix = *it->second;
			for (it = ++it; it != TransformMatrix.end(); ++it){
				CFP_Matrix *= *it->second;
			}
			CFP_Matrixes[i] = CFP_Matrix;
		}
	}

	// Управление камерой
		// Получение углов вращения
		float	Mouse_sensitivity = 0.005,	// Чувствительность мыши
			Angle_horizontal  = Mouse_sensitivity * float(MouseX),
			Angle_vertical    = Mouse_sensitivity * float(MouseY);

		// Ограничение вертикального угла вращения
		float Angle_vertical_max = 0.99 * D3DX_PI / 2.0;
		if (Angle_vertical > Angle_vertical_max){
			Angle_vertical = Angle_vertical_max;
			MouseY = Angle_vertical / Mouse_sensitivity;
		} else if (Angle_vertical < -Angle_vertical_max){
			Angle_vertical = -Angle_vertical_max;
			MouseY = Angle_vertical / Mouse_sensitivity;
		}

		// Кеширование тригонометрических компонент углов вращения
		float	sin_AH = sin(Angle_horizontal), cos_AH = cos(Angle_horizontal),
			sin_AV = sin(Angle_vertical),   cos_AV = cos(Angle_vertical);

		// Получение позиции камеры
		float	PlayerSpeed = 400000, Step = PlayerSpeed * DeltaTime,
			Step_sin = Step * sin_AH, Step_cos = Step * cos_AH;
		if (KeysMap[87]){	// 87 = ord('w') => Вперёд
			Z += Step_cos;
			X += Step_sin;
		}
		if (KeysMap[83]){	// 83 = ord('s') => Назад
			Z -= Step_cos;
			X -= Step_sin;
		}
		if (KeysMap[65]){	// 65 = ord('a') => Влево
			X -= Step_cos;
			Z += Step_sin;
		}
		if (KeysMap[68]){	// 68 = ord('d') => Вправо
			X += Step_cos;
			Z -= Step_sin;
		}

	// Ограничение движения
	float	MadnessComp_Radius2 = MadnessComp_Radius + 0.5, Shahta_Radius2 = Shahta_Radius - 0.5,
		Player_R = sqrt(X*X + Z*Z);
	if (Player_R < 0.001){Player_R = 0.001;}
	float Player_Angle = asin(X / Player_R);
	if (Z < 0){Player_Angle = D3DX_PI - Player_Angle;}
	if (Player_R < MadnessComp_Radius2){Player_R = MadnessComp_Radius2;} else
	if (Player_R > Shahta_Radius2){Player_R = Shahta_Radius2;}
	X = Player_R * sin(Player_Angle);
	Z = Player_R * cos(Player_Angle);

	// Получение матриц вида и камеры
		D3DXMATRIX* _3D_ViewMatrix = new D3DXMATRIX, CFP_CameraMatrix;
		D3DXVECTOR3	position(X, Y, Z),
				target(X + cos_AV * sin_AH, Y + sin_AV, Z + cos_AV * cos_AH),
				up(0, 1, 0);
		D3DXMatrixLookAtLH(_3D_ViewMatrix, &position, &target, &up);
		D3DXMATRIX Matrix;
		D3DXMatrixRotationX(&CFP_CameraMatrix, -Angle_vertical);
		D3DXMatrixRotationY(&Matrix, Angle_horizontal);
		CFP_CameraMatrix *= Matrix;
		D3DXMatrixTranslation(&Matrix, X, Y, Z);
		CFP_CameraMatrix *= Matrix;

	// Стрельба из пулемётов
	if (MouseButtonsMap[mbLeft]){
		Left_MashineGun_SpinSpeed += 1.0E+11 * DeltaTime;
		float Max_SpinSpeed = 1.0E+6;
		if (Left_MashineGun_SpinSpeed > Max_SpinSpeed){
			Left_MashineGun_SpinSpeed = Max_SpinSpeed;
		}
	} else {
		Left_MashineGun_SpinSpeed -= 5.0E+10 * DeltaTime;
		if (Left_MashineGun_SpinSpeed < 0.0){Left_MashineGun_SpinSpeed = 0;}
	}
	int SpinMatrix_Index = PB_2 * 1 + PB * 2;
	D3DXMatrixRotationZ(&Matrix, -Left_MashineGun_SpinSpeed * DeltaTime);
	for (size_t i = 0; i < CFP_Matrixes.size() / 2; i++){
		*CFP_TransformMatrixes[i][SpinMatrix_Index] *= Matrix;
	}
	if (MouseButtonsMap[mbRight]){
		Right_MashineGun_SpinSpeed += 1.0E+11 * DeltaTime;
		float Max_SpinSpeed = 1.0E+6;
		if (Right_MashineGun_SpinSpeed > Max_SpinSpeed){
			Right_MashineGun_SpinSpeed = Max_SpinSpeed;
		}
	} else {
		Right_MashineGun_SpinSpeed -= 5.0E+10 * DeltaTime;
		if (Right_MashineGun_SpinSpeed < 0.0){Right_MashineGun_SpinSpeed = 0;}
	}
	D3DXMatrixRotationZ(&Matrix, Right_MashineGun_SpinSpeed * DeltaTime);
	for (size_t i = CFP_Matrixes.size() / 2; i < CFP_Matrixes.size(); i++){
		*CFP_TransformMatrixes[i][SpinMatrix_Index] *= Matrix;
	}

	// Вращение парящик картинок
	/*TotalTime += 20000.0 * DeltaTime;
	for (size_t i = 0; i < FlyImg_SpinMatrixes.size() / 2; i++){
		D3DXMATRIX SpinMatrix;
		D3DXMatrixRotationY(&SpinMatrix, TotalTime + 2.0 * D3DX_PI * float(i) / float(FlyImg_SpinMatrixes.size() / 2) );
		*FlyImg_SpinMatrixes[i * 2] = SpinMatrix;
		*FlyImg_SpinMatrixes[i * 2 + 1] = SpinMatrix;
	}*/

	// Перерасчёт конечных матриц элементов WFP (World Fixed Points)
	for (std::map<UINT, WFP_Type>::iterator it = WFP.begin(); it != WFP.end(); ++it){
		WFP_Type WFP_piese = it->second;
		if (!WFP_piese.PersistentFlag && WFP_piese.VisibleFlag){
			std::map<UINT, D3DXMATRIX*> TransformMatrix = WFP_piese.TransformMatrixes;
			std::map<UINT, D3DXMATRIX*>::iterator it2 = TransformMatrix.begin();
			D3DXMATRIX WFP_Matrix = *it2->second;
			for (it2 = ++it2; it2 != TransformMatrix.end(); ++it2){
				WFP_Matrix *= *it2->second;
			}
			*(it->second.Matrix) = WFP_Matrix;
		}
	}

	// Рендеринг всего и вся
	D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clBlack, 1.0f, 0);
	D3DDevice->BeginScene();
	D3DDevice->SetTransform(D3DTS_VIEW, _3D_ViewMatrix);
	for (std::map<UINT, WFP_Type>::iterator it = WFP.begin(); it != WFP.end(); ++it){
		WFP_Type WFP_piese = it->second;
		if (WFP_piese.VisibleFlag){
			D3DDevice->SetTransform(D3DTS_WORLD, WFP_piese.Matrix);

			if (WFP_piese.GeometryType == _3D_GT_VertexBuffer){
				_3D_VertexBufferGeometry* VBG = (_3D_VertexBufferGeometry*)WFP_piese.GeometryData;
				D3DDevice->SetFVF(_3D_FVF);
				D3DDevice->SetStreamSource(0, VBG->VertexBuffer, 0, sizeof(_3D_Vertex) );
				D3DDevice->SetMaterial(Base_Material);
				D3DDevice->SetTexture(0, VBG->Texture);
				D3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, VBG->PoligonsCount);
			} else /*if (WFP_piese.GeometryType == _3D_GT_Mesh)*/{
				_3D_MeshGeometry* MeshData = (_3D_MeshGeometry*)WFP_piese.GeometryData;
				for (size_t j = 0; j < MeshData->Materials.size(); j++){
					D3DDevice->SetMaterial(&MeshData->Materials[j]);
					D3DDevice->SetTexture(0, MeshData->Textures[j]);
					MeshData->Mesh->DrawSubset(j);
				}
			}
		}
	}
	D3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	for (size_t i = 0; i < CFP_GeometryLinks.size(); i++){
		if (CFP_VisibleFlag[i]){
			D3DXMATRIX CFP_Matrix = CFP_Matrixes[i];
			if (CFP_use_SharedMatrix){CFP_Matrix *= CFP_SharedMatrix;}
			CFP_Matrix *= CFP_CameraMatrix;
			D3DDevice->SetTransform(D3DTS_WORLD, &CFP_Matrix);

			_3D_GeometryType CFP_Geometry = CFP_GeometryLinks[i];
			if (CFP_Geometry.GeometryType == _3D_GT_VertexBuffer){
				_3D_VertexBufferGeometry* VBG = (_3D_VertexBufferGeometry*)CFP_Geometry.GeometryData;
				D3DDevice->SetFVF(_3D_FVF);
				D3DDevice->SetStreamSource(0, VBG->VertexBuffer, 0, sizeof(_3D_Vertex) );
				D3DDevice->SetMaterial(Base_Material);
				D3DDevice->SetTexture(0, VBG->Texture);
				D3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, VBG->PoligonsCount);
			} else /*if (CFP_Geometry.GeometryType == _3D_GT_Mesh)*/{
				_3D_MeshGeometry* MeshData = (_3D_MeshGeometry*)CFP_Geometry.GeometryData;
				for (size_t j = 0; j < MeshData->Materials.size(); j++){
					D3DDevice->SetMaterial(&MeshData->Materials[j]);
					D3DDevice->SetTexture(0, MeshData->Textures[j]);
					MeshData->Mesh->DrawSubset(j);
				}
			}
		}
	}
	D3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	D3DDevice->SetTransform(D3DTS_VIEW, UI_Base_ViewMatrix);
	D3DDevice->SetFVF(UI_FVF);
	D3DDevice->SetMaterial(Base_Material);
	for (size_t i = 0; i < 0*UI_QuadsLinks.size(); i++){
		if (UI_VisibleFlag[i]){
			D3DDevice->SetStreamSource(0, UI_QuadsLinks[i], 0, sizeof(UI_Vertex) );
			D3DDevice->SetTexture(0, UI_Textures[i]);
			D3DXMATRIX* UI_Matrix = UI_Matrixes[i];
			/* Да, std::vector<D3DXMATRIX*> UI_Matrixes[i] из-за
				домножения на UI_BaseMatrix неоптимально и уж
				лучше тип std::vector<D3DXMATRIX>, но... лень
				переправлять
			*/
			if (UI_use_SharedMatrix){
				UI_Matrix = &(*UI_Matrix * UI_SharedMatrix);
			}
			UI_Matrix = &(*UI_Matrix * UI_BaseMatrix);	// А в конце плоскость UI отодвигается на 1 метр по оси Z.
			D3DDevice->SetTransform(D3DTS_WORLD, UI_Matrix);
			D3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
		}
	}
	D3DDevice->EndScene();
	D3DDevice->Present(NULL, NULL, NULL, NULL);

	FPS++;
	IsRendering = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject*){
	Caption = FPS; FPS = 0;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormKeyDown(TObject*, WORD &Key, TShiftState){
	KeysMap[Key] = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormKeyUp(TObject*, WORD &Key, TShiftState /*Shift*/){
	KeysMap[Key] = false;

	if (Key == VK_ESCAPE){Close();}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormMouseDown(
	TObject*, TMouseButton Button, TShiftState /*Shift*/, int, int)
{
	MouseButtonsMap[Button] = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormMouseUp(
	TObject*, TMouseButton Button, TShiftState /*Shift*/, int, int)
{
	MouseButtonsMap[Button] = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCloseQuery(TObject*, bool &CanClose){
	if (!StopSignal){
		StopSignal	= true;
		CanClose	= false;
	}
}
//---------------------------------------------------------------------------

