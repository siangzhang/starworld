#pragma once
#include "XUI_Wnd.h"

namespace UILib
{
	class XUI_EditBox :	public UIObjTypeT< XUI_Wnd, TypeEditBox >
	{
		friend class LuaBindClass;
		DECLARE_UIMSG_MAP()
		DECLARE_LABLE( EDIT )
	public:
		XUI_EditBox(void);
		~XUI_EditBox(void);

		//�ػ棬ͨ��ʵ��������������ֿռ�����
		virtual void RenderSelf(const CRect& clipper);

		//virtual void OnDestroy();
		//���
		//����ƶ��¼�
		//����˵����
		//pt���������꣬����ڿؼ�
		//sysKeys��������Ҫ������״̬���μ�MSDN	
		virtual	bool onMouseMove(const CPoint& pt, UINT sysKeys);
		virtual bool onMouseHover(const CPoint& pt);
		virtual bool onMouseEnter();
		virtual bool onMouseLeave();
		//��갴���¼�
		//����˵����
		//button�����µļ���0-�����1-�Ҽ���2-�м�
		//pt����������
		//sysKeys��������Ҫ������״̬���μ�MSDN
		virtual	bool onButtonDown(int button, const CPoint& pt, UINT sysKeys);
		virtual	bool onButtonUp(int button, const CPoint& pt, UINT sysKeys);

		//����
		//����˵��
		//keycode�����µļ�
		//sysKeys��������Ҫ������״̬���μ�MSDN
		virtual	bool onKeyDown(uint32 keycode, UINT sysKeys);
		virtual	bool onKeyUp(uint32 keycode, UINT sysKeys);

		//����˵��
		//c��������ַ�
		//sysKeys��������Ҫ������״̬���μ�MSDN
		virtual bool onChar(uint32 c, UINT sysKeys);

		//���뷨
		//�μ�MSDN
		virtual bool onImeComp(uint32 wParam, uint32 lParam);
		virtual bool onImeEndComp(uint32 wParam, uint32 lParam);
		virtual bool onImeNotify(uint32 wParam, uint32 lParam);

		virtual unsigned int OnMoveWindow( CRect& rcWindow );

	protected:
		void RenderCharacter( _tchar szChar, XUI_IFont* pFont, LONG &x, LONG &y, BOOL bRender );
		void DeleteCharacter( size_t nPos );
		bool CaratTimerUpdate( unsigned int handle, unsigned short& repeat, unsigned int& timer );

		void HandleBack();
		void HandleDelete();
		void HandleHome();
		void HandleEnd();
		void HandleWordLeft();
		void HandleCharLeft();
		void HandleWordRight();
		void HandleCharRight();
		void HandleReturn();
		void HandleLineUp();
		void HandleLineDown();

		//---------------------------------------------------------------------//
		// describe	: �����ַ�λ�û���к�
		// nPos		: �ַ�����
		// return	: �к�
		//---------------------------------------------------------------------//
		size_t GetLineFromCharaterPos( size_t nPos )const;

		//---------------------------------------------------------------------//
		// describe	: ������ǰ����ˢ��LineRecorder����
		//---------------------------------------------------------------------//
		void Analyse();

		//---------------------------------------------------------------------//
		// describe	: ���õ�ǰ��
		// line		: ������
		//---------------------------------------------------------------------//
		void SetCurLineNumber( size_t line );
	protected:
		// Attribute
		_string			m_strText;			// �༭������
		bool			m_bWarpText;		// ���б�־

	private:
		typedef _string::size_type Position;
		typedef	std::vector< unsigned int >	line_recorder;
		size_t			m_FirstLineNumber;	// �༭���￴���ĵ�һ���ַ���λ��
		size_t			m_nCurLineNumber;	// ��ǰ������
		size_t			m_CaratPos;			// ���λ��
		line_recorder	m_LineRecorder;		// ���з�λ���б�

		bool	m_bControl, m_bShift;
		CSize	m_WindowSize;				// �Ӵ���С

		unsigned int	m_CaratTimerHandler;
		bool			m_bShowCarat;
	};
}