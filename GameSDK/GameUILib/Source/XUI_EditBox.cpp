#include "GuiHeader.h"
#include "GuiSystem.h"
#include "XUI_Window.h"
#include "XUI_EditBox.h"

namespace UILib
{
	BEGIN_UIMSG_MAP( XUI_EditBox, XUI_Wnd )
	END_UIMSG_MAP()

	XUI_EditBox::XUI_EditBox(void)
	: UIObjTypeT< XUI_Wnd, TypeEditBox >()
	, m_bControl( false )
	, m_bShift( false )
	, m_CaratPos( 0 )
	, m_FirstLineNumber( 0 )
	, m_nCurLineNumber( 0 )
	, m_bShowCarat( true )
	, m_dwBorderColor( XUI_ARGB(0xff,0x80,0x80,0x80) )
	, m_dwBackgroundColor( XUI_ARGB(0x30,0x20,0x20,0x20) )
	{
		XUI_IFont* pFont = m_pFont?m_pFont:GuiSystem::Instance().GetDefaultFont();
		if( pFont )
		{
			m_WindowSize.cx		= m_WindowRect.Width()/pFont->GetCharacterWidth( _T(' ') );
			m_WindowSize.cy		= m_WindowRect.Height()/pFont->GetCharacterHeight();
		}

		m_CaratTimerHandler = GuiSystem::Instance().SetTimer( event_function( this, &XUI_EditBox::CaratTimerUpdate ), 1, TIMER_SECOND(0.5f) );
		m_text.push_back(_T(""));
	}

	XUI_EditBox::~XUI_EditBox(void)
	{
		GuiSystem::Instance().KillTimer( m_CaratTimerHandler );
	}

	bool XUI_EditBox::CaratTimerUpdate( unsigned int handle, unsigned short& repeat, unsigned int& timer )
	{
		m_bShowCarat = !m_bShowCarat;
		repeat = 1;
		return true;
	}

	size_t XUI_EditBox::NaturalLine( size_t nLine )
	{
		if( nLine >= m_text.size() ) return 0;

		line& l = m_text[nLine];
		size_t ret = 0;
		text::iterator i = m_text.begin() + nLine + 1;
		while( l.type == type_r && i != m_text.end() )
		{
			l.append( *i );
			l.type = (*i).type;
			i = m_text.erase( i );
			++ret;
		}

		return ret;
	}

	void XUI_EditBox::SetText( const std::string &t )
	{
		m_text.clear();
		m_text.push_back(_T(""));
		std::string::size_type begin = 0, end = 0;
		while( ( end = t.find( _T('\n'), begin ) ) != std::string::npos )
		{
			m_text.push_back( XA2T( t.substr( begin, end ) ) );
			++begin;
		}
	}

	std::string XUI_EditBox::GetText()const
	{
		std::string t;
		text::const_iterator i = m_text.begin();
		while( i != m_text.end() )
		{
			t.append( XT2A( *i ) );
			++i;
		}

		return t;
	}

	void XUI_EditBox::RenderCharacter( _tchar szChar, XUI_IFont* pFont, LONG &x, LONG &y, BOOL bRender )
	{
		if( bRender )
		{
			XUI_DrawCharacter( szChar, pFont, (float)x, (float)y );
		}

		x += pFont->GetCharacterWidth( szChar );
	}

	//�ػ棬ͨ��ʵ��������������ֿռ�����
	void XUI_EditBox::RenderSelf(const x_rect& clipper)
	{
		x_point pt = m_WindowRect.TopLeft();
		AdjustPoint( pt, true );

		XUI_DrawRect( m_WindowRect, m_dwBorderColor, m_dwBackgroundColor );

		x_rect rc;
		rc.IntersectRect( m_WindowRect, clipper );

		x_point CharPos = pt;
		x_point CaratPos;
		XUI_IFont* pFont = m_pFont?m_pFont:GuiSystem::Instance().GetDefaultFont();

		for( Position i = m_FirstLineNumber; i < m_text.size(); ++i )
		{
			line& l = m_text[i];
			end_type t = l.type;

			if( CharPos.y > rc.bottom - pFont->GetCharacterHeight() )
			{
				// �����߶�������
				break;
			}

			for( size_t cursor = 0; cursor <= l.size(); ++cursor )
			{
				_tchar c = l[cursor];

				if( pFont->GetCharacterWidth( c ) + CharPos.x > rc.right )
				{
					if( m_bWarpText )
					{
						// �ַ���ʾ�������ȣ���������ʾ
						// ��û�л�����Ϊ�����Ĵ����ӵ���һ��
						line ll( l.substr( cursor - 1, -1 ), l.type );
						l.erase( cursor - 1, -1 );
						l.type = type_r;
						l.cursor_position = l.size();

						size_t lsize = l.size();
						m_text.insert( m_text.begin() + i + 1, ll );

						// ���ݹ��λ���ж��Ƿ�Ҫ�����趨��������С�
						if( m_nCurLineNumber == i && m_CaratPos > lsize )
						{
							SetCurLineNumber( m_nCurLineNumber + 1 );
							m_CaratPos = cursor - lsize;
						}
						break;
					}
					else
					{
						// �����д���һ�п�ʼ
						CharPos.x = pt.x;
						break;
					}
				}

				// �ж��Ƿ񱻻��ơ�
				BOOL bRender = rc.PtInRect( CharPos + x_point( pFont->GetCharacterWidth( c ), pFont->GetCharacterHeight() ) );
				if( _istprint( c ) )
				{
					// ����ʾ�ַ�
					RenderCharacter( c, pFont, CharPos.x, CharPos.y, bRender );
				}
				else if( c == _T('\n') )
				{
					break;
				}
				else
				{
					switch( c )
					{
					case '\t':
						{
							// �Ʊ�����
							size_t NextTab = CharPos.x + (4 - cursor%4)*pFont->GetCharacterWidth( _T(' ') );
							if( (long)NextTab > pt.x + m_WindowRect.Width() ) NextTab = pt.x + m_WindowRect.Width();
							while( CharPos.x < (long)NextTab )
							{
								RenderCharacter( _T(' '), pFont, CharPos.x, CharPos.y, bRender );
							}
						}
						break;
					case '\0':
						// Ϊ���ƹ��ռλ�á�
						RenderCharacter( _T(' '), pFont, CharPos.x, CharPos.y, bRender );
						break;
					default:
						RenderCharacter( _T('?'), pFont, CharPos.x, CharPos.y, bRender );
						break;
					}
				}

				if( i == m_nCurLineNumber && cursor == m_CaratPos )
				{
					long x = CaratPos.x = CharPos.x - long( pFont->GetCharacterWidth( _T('|') )*1.5f );
					long y = CaratPos.y = CharPos.y;
					if( m_bShowCarat )
					{
						// �Ƿ���ƹ��
						RenderCharacter( _T('|'), pFont, x, y, bRender );
					}
				}

			}

			// ���С�
			CharPos.x = pt.x;
			CharPos.y += pFont->GetCharacterHeight();
		}

		if( m_bFocused && !m_Candlist.strBuffer.empty() )
		{
			XUI_Window* pWnd = GuiSystem::Instance().GetDesktop( DEFAULT_DESKTOP );
			const x_rect& rcWindow = pWnd->GetWindowRect();
			CaratPos.x = pFont->GetCharacterWidth( _T(' ') ) + ( (CaratPos.x + m_Candlist.rcCandidate.Width() > rcWindow.Width())?rcWindow.Width()-m_Candlist.rcCandidate.Width()-1:CaratPos.x);
			if( CaratPos.x < 0 ) CaratPos.x = 0;

			CaratPos.y = pFont->GetCharacterHeight()/2 + ( (CaratPos.y + m_Candlist.rcCandidate.Height() > rcWindow.Height())?rcWindow.Height()-m_Candlist.rcCandidate.Height()-1:CaratPos.y );
			if( CaratPos.y < 0 ) CaratPos.y = 0;

			XUI_DrawRect( 
				x_rect( CaratPos.x, CaratPos.y, CaratPos.x + m_Candlist.rcCandidate.Width(), CaratPos.y + pFont->GetCharacterHeight() + 2 ),
				m_dwBorderColor, 
				m_dwBackgroundColor );
			XUI_DrawText( m_Candlist.strBuffer.c_str(), pFont, float( CaratPos.x + 1 ), float( CaratPos.y + 1 ) );

			if( m_Candlist.bShowWindow )
			{
				CaratPos.y += pFont->GetCharacterHeight() + 2;
				XUI_DrawRect( 
					x_rect( CaratPos.x, CaratPos.y, CaratPos.x + m_Candlist.rcCandidate.Width(), CaratPos.y + m_Candlist.rcCandidate.Height() ),
					m_dwBorderColor, 
					m_dwBackgroundColor );
			}
		}
	}

	//���
	//����ƶ��¼�
	//����˵����
	//pt���������꣬����ڿؼ�
	//sysKeys��������Ҫ������״̬���μ�MSDN	
	bool XUI_EditBox::onMouseMove(const x_point& pt, UINT sysKeys)
	{
		return false;
	}

	bool XUI_EditBox::onMouseHover(const x_point& pt)
	{
		return true;
	}

	bool XUI_EditBox::onMouseEnter()
	{
		return true;
	}

	bool XUI_EditBox::onMouseLeave()
	{
		return true;
	}

	//��갴���¼�
	//����˵����
	//button�����µļ���0-�����1-�Ҽ���2-�м�
	//pt����������
	//sysKeys��������Ҫ������״̬���μ�MSDN
	bool XUI_EditBox::onButtonDown(int button, const x_point& pt, UINT sysKeys)
	{
		return true;
	}

	bool XUI_EditBox::onButtonUp(int button, const x_point& pt, UINT sysKeys)
	{
		return true;
	}

	//����
	//����˵��
	//keycode�����µļ�
	//sysKeys��������Ҫ������״̬���μ�MSDN
	bool XUI_EditBox::onKeyDown(uint32 keycode, UINT sysKeys)
	{
		switch( keycode )
		{
		case VK_CONTROL:
			m_bControl = true;
			break;
		case VK_SHIFT:
		case VK_RSHIFT:
			m_bShift = true;
			break;
		case VK_BACK:
			HandleBack();
			break;
		case VK_DELETE:
			HandleDelete();
			break;
		case VK_HOME:
			HandleHome();
			break;
		case VK_END:
			HandleEnd();
			break;
		case VK_LEFT:
			if( m_bControl )
				HandleWordLeft();
			else
				HandleCharLeft();
			break;
		case VK_RIGHT:
			if( m_bControl )
				HandleWordRight();
			else
				HandleCharRight();
			break;
		case VK_UP:
			HandleLineUp();
			break;
		case VK_DOWN:
			HandleLineDown();
			break;
		case VK_RETURN:
			HandleReturn();
			break;
		}
		return true;
	}

	void XUI_EditBox::DeleteCharacter( size_t nLine, size_t nPos, size_t nCount )
	{
		Position n = nLine;
		Position p = nPos;
		while( nCount )
		{
			line& l = m_text.at( n );
			if( l.size() - p < nCount )
			{
				l.erase( p, l.size() - p );
				nCount -= ( l.size() - p );

				// �к����У�����һ�к͵�ǰ�кϲ�
				if( m_text.size() > n + 1 )
				{
					if( l.type == type_n ) --nCount;
					l.type = type_r;

					NaturalLine( m_nCurLineNumber );
				}
				else
				{
					break;
				}
			}
			else if( l.type == type_r )
			{
				NaturalLine( m_nCurLineNumber );
			}
			else
			{
				l.erase( p, nCount );
				nCount = 0;
			}
		}
	}

	void XUI_EditBox::HandleBack()
	{
		if( m_CaratPos <= 0 )
		{
			if( m_nCurLineNumber == 0 ) return;
			SetCurLineNumber( m_nCurLineNumber - 1 );
			line l = m_text.at( m_nCurLineNumber );
			m_CaratPos = l.size() - (size_t)l.type;
		}
		else
		{
			--m_CaratPos;
		}
		DeleteCharacter( m_nCurLineNumber, m_CaratPos, 1 );
	}

	void XUI_EditBox::HandleDelete()
	{
		DeleteCharacter( m_nCurLineNumber, m_CaratPos, 1 );
	}

	void XUI_EditBox::HandleHome()
	{
		m_CaratPos = 0;
	}

	void XUI_EditBox::HandleEnd()
	{
		m_CaratPos = m_text.at( m_nCurLineNumber ).size();
	}

	void XUI_EditBox::HandleWordLeft()
	{
		//while( m_CaratPos > 0 && m_strText[m_CaratPos] != _T(' ') )
		//{
		//	--m_CaratPos;
		//	if( m_nCurLineNumber > 0 && m_CaratPos < m_LineRecorder[m_nCurLineNumber-1] )
		//	{
		//		SetCurLineNumber( m_nCurLineNumber - 1 );
		//	}
		//}
	}

	void XUI_EditBox::HandleCharLeft()
	{
		if( m_CaratPos > 0 )
		{
			--m_CaratPos; 
		}
		else
		{
			SetCurLineNumber( m_nCurLineNumber - 1 );
			HandleEnd();
		}
	}

	void XUI_EditBox::HandleWordRight()
	{
		//while( m_CaratPos < m_strText.length() && m_strText[m_CaratPos] != _T(' ') )
		//{
		//	++m_CaratPos;
		//	if( m_nCurLineNumber < m_LineRecorder.size() && m_CaratPos >= m_LineRecorder[m_nCurLineNumber] )
		//	{
		//		SetCurLineNumber( m_nCurLineNumber + 1 );
		//	}
		//}
	}

	void XUI_EditBox::HandleCharRight()
	{
		line &l = m_text.at( m_nCurLineNumber );
		if( m_CaratPos < l.size() )
		{
			++m_CaratPos;
		}
		else
		{
			SetCurLineNumber( m_nCurLineNumber + 1 );
			HandleHome();
		}
	}

	void XUI_EditBox::HandleReturn()
	{
		line& l = m_text[m_nCurLineNumber];

		line ll = l.substr( m_CaratPos, l.size() - m_CaratPos );
		ll.type = (m_CaratPos==l.size()?type_n:l.type);

		l.erase( l.begin() + m_CaratPos, l.end() );
		l.type = type_n;

		m_text.insert( m_text.begin() + m_nCurLineNumber + 1, ll );
		NaturalLine( m_nCurLineNumber + 1 );

		m_CaratPos = 0;
		SetCurLineNumber( m_nCurLineNumber + 1 );
	}

	void XUI_EditBox::HandleLineUp()
	{
		if( m_nCurLineNumber > 0 )
		{
			SetCurLineNumber( m_nCurLineNumber - 1 );
		}
	}

	void XUI_EditBox::HandleLineDown()
	{
		SetCurLineNumber( m_nCurLineNumber + 1 );
	}

	bool XUI_EditBox::onKeyUp( uint32 keycode, UINT sysKeys )
	{
		switch( keycode )
		{
		case VK_CONTROL:
			m_bControl = false;
			break;
		case VK_SHIFT:
		case VK_RSHIFT:
			m_bShift = false;
			break;
		}
		return true;
	}

	//����˵��
	//c��������ַ�
	//sysKeys��������Ҫ������״̬���μ�MSDN
	bool XUI_EditBox::onChar(uint32 c, UINT sysKeys)
	{
		if( _istprint( LOWORD(c) ) )
		{
			m_text.at(m_nCurLineNumber).insert( m_CaratPos++, 1, (wchar_t)c );
			NaturalLine(m_nCurLineNumber);
		}
		return true;
	}

	//���뷨
	//�μ�MSDN
	bool XUI_EditBox::onImeComp(uint32 wParam, uint32 lParam)
	{
		return true;
	}

	bool XUI_EditBox::onImeEndComp(uint32 wParam, uint32 lParam)
	{
		return true;
	}

	bool XUI_EditBox::onImeNotify(uint32 wParam, uint32 lParam)
	{
		return true;
	}

	unsigned int XUI_EditBox::OnMoveWindow( x_rect& rcWindow )
	{
		XUI_IFont* pFont = m_pFont?m_pFont:GuiSystem::Instance().GetDefaultFont();
		if( pFont )
		{
			m_WindowSize.cx		= m_WindowRect.Width()/pFont->GetCharacterWidth( _T(' ') );
			m_WindowSize.cy		= m_WindowRect.Height()/pFont->GetCharacterHeight();
		}
		Analyse();
		return 0;
	}

	//---------------------------------------------------------------------//
	// describe	: ������ǰ����ˢ��LineRecorder����
	//---------------------------------------------------------------------//
	void XUI_EditBox::Analyse()
	{

	}

	//---------------------------------------------------------------------//
	// describe	: ���õ�ǰ��
	// line		: ������
	//---------------------------------------------------------------------//
	void XUI_EditBox::SetCurLineNumber( size_t nLine )
	{
		if( nLine >= m_text.size() ) return;

		line &l = m_text.at( m_nCurLineNumber );
		l.cursor_position = m_CaratPos;

		m_nCurLineNumber = nLine;
		line &curline = m_text.at( m_nCurLineNumber );
		m_CaratPos = curline.cursor_position;


		if( m_nCurLineNumber < m_FirstLineNumber )
		{
			m_FirstLineNumber = ( m_FirstLineNumber <= (size_t)m_WindowSize.cy?0:(m_FirstLineNumber - m_WindowSize.cy) );
		}
		else if( m_nCurLineNumber - m_FirstLineNumber >= (size_t)m_WindowSize.cy )
		{
			m_FirstLineNumber = m_nCurLineNumber;
		}
	}

	LRESULT XUI_EditBox::OnWndMsg( UINT nMsg, WPARAM wParam, LPARAM lParam )
	{
		switch( nMsg )
		{
		case WM_INPUTLANGCHANGE:
			break;
		case WM_IME_SETCONTEXT:
			break;
		case WM_IME_SELECT:
			break;
		}
		return XUI_Wnd::OnWndMsg( nMsg, wParam, lParam );
	}
}