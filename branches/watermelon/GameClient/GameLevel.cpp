#include "StdAfx.h"
#include "GameLevel.h"

CGameLevel::CGameLevel( b2World* world )
: m_world( world )
, hge( hgeCreate(HGE_VERSION) )
, m_Background( 0, 0, 0, 0, 0 )
{
}

CGameLevel::~CGameLevel(void)
{
	hge->Release();
}

bool CGameLevel::Load( const char* filename )
{
	m_hBackground = hge->Texture_Load( "background.png" );
	m_Background.SetTexture(m_hBackground);
	m_Background.SetTextureRect( 0, 0, hge->Texture_GetWidth(m_hBackground), hge->Texture_GetHeight(m_hBackground) );

	b2Body* ground = NULL;
	{
		b2BodyDef bd;
		ground = m_world->CreateBody(&bd);

		b2EdgeShape shape;
		shape.Set(b2Vec2(-20.0f, 0.0f), b2Vec2(20.0f, 0.0f));

		b2FixtureDef fd;
		fd.shape = &shape;

		ground->CreateFixture(&fd);
	}

	b2Body* attachment;
	// Define attachment
	{
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(0.0f, 3.0f);
		attachment = m_world->CreateBody(&bd);

		b2PolygonShape shape;
		shape.SetAsBox(0.5f, 2.0f);
		attachment->CreateFixture(&shape, 2.0f);
	}

	b2Body* platform;
	// Define platform
	{
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(-4.0f, 5.0f);
		platform = m_world->CreateBody(&bd);

		b2PolygonShape shape;
		shape.SetAsBox(0.5f, 4.0f, b2Vec2(4.0f, 0.0f), 0.5f * b2_pi);

		b2FixtureDef fd;
		fd.shape = &shape;
		fd.friction = 0.6f;
		fd.density = 2.0f;
		platform->CreateFixture(&fd);

		b2RevoluteJointDef rjd;
		rjd.Initialize(attachment, platform, b2Vec2(0.0f, 5.0f));
		rjd.maxMotorTorque = 50.0f;
		rjd.enableMotor = true;
		m_world->CreateJoint(&rjd);

		b2PrismaticJointDef pjd;
		pjd.Initialize(ground, platform, b2Vec2(0.0f, 5.0f), b2Vec2(1.0f, 0.0f));

		pjd.maxMotorForce = 1000.0f;
		pjd.enableMotor = true;
		pjd.lowerTranslation = -10.0f;
		pjd.upperTranslation = 10.0f;
		pjd.enableLimit = true;

		(b2PrismaticJoint*)m_world->CreateJoint(&pjd);
	}

	// Create a payload
	{
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(0.0f, 28.0f);
		b2Body* body = m_world->CreateBody(&bd);

		b2PolygonShape shape;
		shape.SetAsBox(0.75f, 0.75f);

		b2FixtureDef fd;
		fd.shape = &shape;
		fd.friction = 0.6f;
		fd.density = 2.0f;

		body->CreateFixture(&fd);
	}

	platform->SetType(b2_kinematicBody);
	platform->SetLinearVelocity(b2Vec2(-2.0f, 0.0f));
	platform->SetAngularVelocity(0.0f);

	return true;
}

bool CGameLevel::UpdateLogic( float fDelta )
{
	return true;
}

void CGameLevel::Render()
{
	m_Background.RenderStretch( 0, 0, hge->System_GetState( HGE_SCREENWIDTH ), hge->System_GetState( HGE_SCREENHEIGHT )  );
}

void CGameLevel::Destroy()
{

}