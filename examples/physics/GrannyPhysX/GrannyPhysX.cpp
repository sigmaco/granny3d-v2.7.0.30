// =====================================================================
// GrannyPhysX: A helper layer for integrating Granny with the Ageia
//  PhysX SDK.
//
// See header file for detailed commentary
// =====================================================================
#include "granny.h"


#include "GrannyPhysX.h"
#include "NXU_Stream.h"
#include "NXU_Helper.h"


#if !NOGRANNY

#include <vector>


// Our mapping object.  Implementation of the non-trivial functions
// can be found below the forwarding layer at the bottom of this file.
class GrannyPhysX : public NXU_userNotify, public NXU_errorReport
{
public:
    GrannyPhysX();
    ~GrannyPhysX();
    bool IsValid() const;

    bool createActorBinding(NXU::NxuPhysicsCollection*  Collection,
                            NxPhysicsSDK*          SDK,
                            NxScene*               Scene,
                            granny_model_instance* ModelInstance);

    bool setWorldPose(granny_world_pose const* WorldPose);
    bool setWorldPoseForBone(granny_world_pose const* WorldPose,
                             int const                BoneIndex);

    bool moveWorldPoseForBone(granny_world_pose const* WorldPose,
							  int const                BoneIndex);

    bool getWorldPose(granny_world_pose*       WorldPose,
                      granny_local_pose const* LocalPose,
					  float const*             Offset4x4);

    int getNumBones() const;
    int getNumActors() const;

	NxActor* getActor(int const ActorIndex);
	NxActor* getActorByName(char const* ActorName);
	NxActor* getActorForBone(int const BoneIndex);

    // Implements the NxuUserNotify interface so we can capture
    // pointers to the instantiated objects.
public:

    virtual void NXU_errorMessage(bool severity,const char *str)
    {
    }

    virtual void NXU_notifyScene(NxScene *scene,const char *userProperties) { m_scene = scene; }
    virtual void NXU_notifyFluid(NxFluid *fluid,const char *userProperties) {}
    virtual void NXU_notifyJoint(NxJoint *joint,const char *userProperties)
    {
        m_joints.push_back(joint);
    }
    virtual void NXU_notifyActor(NxActor *actor,const char *userProperties)
    {
      const char* ActorName = actor->getName();
      m_actors.push_back(actor);
    }

		virtual void NXU_notifyCloth(NxCloth *c,const char *userProperteis) { };


    NxScene * NXU_preNotifyScene(unsigned int sno,NxSceneDesc &scene,const char *userProperties)
    {
    	return 0;
    }

    bool      NXU_preNotifyJoint(NxJointDesc &joint,const char *userProperties)
    {
    	return true;
    }

    bool      NXU_preNotifyActor(NxActorDesc &actor,const char *userProperties)
    {
    	return true;
    }

    bool      NXU_preNotifyCloth(NxClothDesc &cloth,const char *userProperties)
    {
    	return false; // not implemented yet
    }

    bool      NXU_preNotifyFluid(NxFluidDesc &fluid,const char *userProperties)
    {
    	return false;
    }


private:
	  NxScene               *m_scene;
    granny_model_instance* m_modelInstance;
    granny_model*          m_sourceModel;
    granny_skeleton*       m_sourceSkeleton;

    std::vector<NxActor*>  m_actors;
    std::vector<NxJoint*>  m_joints;
    std::vector<int>       m_actorToBoneMap;
    std::vector<int>       m_boneToActorMap;
};

GrannyPhysX* grp_createMapping(NXU::NxuPhysicsCollection*  Collection,
							   NxPhysicsSDK*          SDK,
							   NxScene*               Scene,
							   granny_model_instance* ModelInstance)
{
    assert(Collection);
    assert(SDK);
    assert(ModelInstance);

    GrannyPhysX *NewMapping = new GrannyPhysX;
    if (!NewMapping->createActorBinding(Collection, SDK, Scene, ModelInstance))
    {
        delete NewMapping;
        NewMapping = NULL;
    }
    else
    {
        assert(NewMapping->IsValid());
    }

    return NewMapping;
}

void grp_releaseMapping(GrannyPhysX* Mapping)
{
    if (Mapping)
    {
        assert(Mapping->IsValid());
        delete Mapping;
    }
}

bool grp_setWorldPose(GrannyPhysX*             Mapping,
                      granny_world_pose const* WorldPose)
{
    assert(Mapping);
    return Mapping->setWorldPose(WorldPose);
}

bool grp_setWorldPoseForBone(GrannyPhysX*             Mapping,
                             granny_world_pose const* WorldPose,
                             const int                BoneIndex)
{
    assert(Mapping);

    return Mapping->setWorldPoseForBone(WorldPose, BoneIndex);
}

bool grp_moveWorldPoseForBone(GrannyPhysX*             Mapping,
							  granny_world_pose const* WorldPose,
							  const int                BoneIndex)
{
    assert(Mapping);

    return Mapping->moveWorldPoseForBone(WorldPose, BoneIndex);
}

bool grp_getWorldPose(GrannyPhysX*             Mapping,
                      granny_world_pose*       WorldPose,
                      granny_local_pose const* LocalPose,
                      float const*             Offset4x4)
{
    assert(Mapping);
    assert(WorldPose);
    assert(LocalPose);

    return Mapping->getWorldPose(WorldPose, LocalPose, Offset4x4);
}

int grp_getNumActors(GrannyPhysX* Mapping)
{
    assert(Mapping);
	
    return Mapping->getNumActors();
}

NxActor* grp_getActor(GrannyPhysX* Mapping,
                      int const    ActorIndex)
{
    assert(Mapping);
    assert(ActorIndex >= 0 && ActorIndex < Mapping->getNumActors());

    return Mapping->getActor(ActorIndex);
}

NxActor* grp_findActorByName(GrannyPhysX* Mapping,
							 const char*  ActorName)
{
    assert(Mapping);
    assert(ActorName);

    return Mapping->getActorByName(ActorName);
}

NxActor* grp_findActorForBone(GrannyPhysX* Mapping,
							  int const    BoneIndex)
{
    assert(Mapping);
    assert(BoneIndex >= 0 && BoneIndex < Mapping->getNumBones());

    return Mapping->getActorForBone(BoneIndex);
}


// =====================================================================
// Retrieves the actor that corresponds to the given bone.  Returns
// NULL if no such corresponce exists.
NxActor* grp_findActorForBone(GrannyPhysX* Mapping,
							  int const BoneIndex);



// =====================================================================
// =====================================================================
GrannyPhysX::GrannyPhysX()
    : m_sourceSkeleton(NULL),
      m_sourceModel(NULL),
			m_scene(NULL),
      m_modelInstance(NULL)
{
    //
}

GrannyPhysX::~GrannyPhysX()
{
    for (size_t i = 0; i < m_joints.size(); i++)
    {
        NxScene& scene = m_joints[i]->getScene();
        scene.releaseJoint(*m_joints[i]);
    }
        
    for (size_t i = 0; i < m_actors.size(); i++)
    {
        NxScene& scene = m_actors[i]->getScene();
        scene.releaseActor(*m_actors[i]);
    }

    m_sourceSkeleton  = NULL;
    m_sourceModel     = NULL;
    m_modelInstance   = NULL;
}

bool GrannyPhysX::IsValid() const
{
	if (m_sourceSkeleton == NULL || m_sourceModel == NULL || m_modelInstance == NULL)
		return false;

	if (m_actors.size() != m_actorToBoneMap.size())
		return false;

	if (granny_int32x(m_boneToActorMap.size()) != m_sourceSkeleton->BoneCount)
		return false;

	{for(size_t Actor = 0; Actor < m_actors.size(); ++Actor)
	{
		if (m_actors[Actor] == NULL)
			return false;
	}}

	{for(size_t Joint = 0; Joint < m_joints.size(); ++Joint)
	{
		if (m_joints[Joint] == NULL)
			return false;
	}}

	{for(size_t A2B = 0; A2B < m_actorToBoneMap.size(); ++A2B)
	{
		if (m_actorToBoneMap[A2B] == GRP_NO_MAPPING)
			continue;

		if (m_actorToBoneMap[A2B] < 0 || m_actorToBoneMap[A2B] >= m_sourceSkeleton->BoneCount)
			return false;
	}}
	{for(size_t B2A = 0; B2A < m_boneToActorMap.size(); ++B2A)
	{
		if (m_boneToActorMap[B2A] == GRP_NO_MAPPING)
			continue;

		if (m_boneToActorMap[B2A] < 0 || m_boneToActorMap[B2A] >= int(m_actors.size()))
			return false;
	}}

	return true;
}

bool GrannyPhysX::createActorBinding(NXU::NxuPhysicsCollection*  Collection,
                                     NxPhysicsSDK*          SDK,
                                     NxScene*               Scene,
                                     granny_model_instance* ModelInstance)
{
  // We shouldn't already be bound...
  assert(!IsValid());
  assert(Collection);
  assert(SDK);
  assert(ModelInstance);

	// Capture the granny state
	m_modelInstance  = ModelInstance;
	m_sourceModel    = GrannyGetSourceModel(m_modelInstance);
	m_sourceSkeleton = GrannyGetSourceSkeleton(m_modelInstance);

	bool ok = NXU::instantiateCollection(Collection, *SDK, Scene, 0, this );

	if ( Scene == NULL )
	  Scene = m_scene;

	// Compute the mapping arrays.
	assert(m_actorToBoneMap.size() == 0 && m_boneToActorMap.size() == 0);
  m_actorToBoneMap.resize(m_actors.size(), GRP_NO_MAPPING);
  m_boneToActorMap.resize(m_sourceSkeleton->BoneCount, GRP_NO_MAPPING);

	{for(size_t ActorIdx = 0; ActorIdx < m_actors.size(); ++ActorIdx)
	{
        assert(m_actors[ActorIdx]);
        const char* ActorName = m_actors[ActorIdx]->getName();

        granny_int32x BoneIndex;
        if (GrannyFindBoneByName(m_sourceSkeleton, ActorName, &BoneIndex))
        {
            m_boneToActorMap[BoneIndex] = int(ActorIdx);
            m_actorToBoneMap[ActorIdx]  = BoneIndex;
        }
	}}

	return IsValid();
}

bool GrannyPhysX::setWorldPose(granny_world_pose const* WorldPose)
{
	assert(IsValid());
	assert(WorldPose);
	assert(GrannyGetWorldPoseBoneCount(WorldPose) >= m_sourceSkeleton->BoneCount);

    granny_matrix_4x4* WorldMatrices = GrannyGetWorldPose4x4Array(WorldPose);
	for (granny_int32x i = 0; i < m_sourceSkeleton->BoneCount; i++)
    {
        if (m_boneToActorMap[i] == GRP_NO_MAPPING)
            continue;

        NxActor* Actor = m_actors[m_boneToActorMap[i]];
        assert(Actor);

        NxMat34 globalPose;
        globalPose.setColumnMajor44((float*)WorldMatrices[i]);
        Actor->setGlobalPose(globalPose);
    }

    return true;
}

bool GrannyPhysX::setWorldPoseForBone(granny_world_pose const* WorldPose,
									  int const                BoneIndex)
{
	assert(IsValid());
	assert(WorldPose);
	assert(BoneIndex >= 0 && BoneIndex < m_sourceSkeleton->BoneCount);
	assert(BoneIndex < GrannyGetWorldPoseBoneCount(WorldPose));

	if (m_boneToActorMap[BoneIndex] != GRP_NO_MAPPING)
	{
		NxActor* pActor = m_actors[m_boneToActorMap[BoneIndex]];
		granny_real32* WorldMatrix = GrannyGetWorldPose4x4(WorldPose, BoneIndex);
		assert(pActor && WorldMatrix);
		
		NxMat34 globalPose;
		globalPose.setColumnMajor44(WorldMatrix);
		pActor->setGlobalPose(globalPose);
		return true;
	}
	else
	{
		return false;
	}
}

bool GrannyPhysX::moveWorldPoseForBone(granny_world_pose const* WorldPose,
									   int const                BoneIndex)
{
	assert(IsValid());
	assert(WorldPose);
	assert(BoneIndex >= 0 && BoneIndex < m_sourceSkeleton->BoneCount);
	assert(BoneIndex < GrannyGetWorldPoseBoneCount(WorldPose));

	if (m_boneToActorMap[BoneIndex] != GRP_NO_MAPPING)
	{
		NxActor* pActor = m_actors[m_boneToActorMap[BoneIndex]];
		granny_real32* WorldMatrix = GrannyGetWorldPose4x4(WorldPose, BoneIndex);
		assert(pActor && WorldMatrix);
		
		NxMat34 globalPose;
		globalPose.setColumnMajor44(WorldMatrix);
		pActor->moveGlobalPose(globalPose);
		return true;
	}
	else
	{
		return false;
	}
}

bool GrannyPhysX::getWorldPose(granny_world_pose*       WorldPose,
                               granny_local_pose const* LocalPose,
                               float const*             Offset4x4)
{
	assert(IsValid());
    assert(WorldPose);
    assert(LocalPose);
    assert(GrannyGetWorldPoseBoneCount(WorldPose) >= m_sourceSkeleton->BoneCount);
    assert(GrannyGetLocalPoseBoneCount(LocalPose) >= m_sourceSkeleton->BoneCount);

	// If NULL was set as the world offset, use an identity matrix...
	if (Offset4x4 == NULL)
	{
		static const float StaticIdentity[16] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		Offset4x4 = StaticIdentity;
	}

	// We do this in two passes.  First, we extract the actor matrices
	// for bones that have a direct correspondence in the physics
	// model.  Then, using the local pose, we fill in the world
	// matrices for bones that don't have a representation in the
	// model.
	
    granny_matrix_4x4* WorldMatrices = GrannyGetWorldPose4x4Array(WorldPose);
	{for(size_t ActorIdx = 0; ActorIdx < m_actors.size(); ++ActorIdx)
	{
        if (m_actorToBoneMap[ActorIdx] == GRP_NO_MAPPING)
            continue;

        NxActor* Actor = m_actors[ActorIdx];
        assert(Actor);

        NxMat34 globalPose = Actor->getGlobalPose();
        globalPose.getColumnMajor44((float*)WorldMatrices[m_actorToBoneMap[ActorIdx]]);
	}}

	{for(granny_int32x BoneIdx = 0; BoneIdx < m_sourceSkeleton->BoneCount; ++BoneIdx)
	{
		// If we already have set this bone's matrix through the
		// mapping, ignore it
        if (m_boneToActorMap[BoneIdx] != GRP_NO_MAPPING)
            continue;

        granny_matrix_4x4* Bone4x4 = &WorldMatrices[BoneIdx];

        granny_transform* pLocalXForm = GrannyGetLocalPoseTransform(LocalPose, BoneIdx);
		granny_real32 LocalComposite[16];
		GrannyBuildCompositeTransform4x4(pLocalXForm, LocalComposite);

        granny_bone& Bone = m_sourceSkeleton->Bones[BoneIdx];
		float const* Parent4x4;
        if (Bone.ParentIndex != GrannyNoParentBone)
        {
            Parent4x4 = (granny_real32*)&WorldMatrices[Bone.ParentIndex];
        }
        else
        {
			Parent4x4 = Offset4x4;
        }

		GrannyColumnMatrixMultiply4x4((granny_real32*)Bone4x4,
									  LocalComposite, Parent4x4);
	}}

	// Finally, since we've only computed the bones' World4x4, we use
	// BuildCompositeFromWorldPose to fill in the skinning matrices.
	// We might want to make this computation optional, since we only
	// need the composite matrices if the world_pose will be directly
	// used for skinning.
    if (GrannyGetWorldPoseComposite4x4Array(WorldPose) != NULL)
	{
		GrannyBuildWorldPoseComposites(m_sourceSkeleton,
									   0, m_sourceSkeleton->BoneCount,
									   WorldPose);
	}

    return true;
}

int GrannyPhysX::getNumBones() const
{
	assert(IsValid());

	return m_sourceSkeleton->BoneCount;
}

int GrannyPhysX::getNumActors() const
{
	assert(IsValid());

	return int(m_actors.size());
}

NxActor* GrannyPhysX::getActor(int const ActorIndex)
{
	assert(IsValid());
	assert(ActorIndex >= 0 && ActorIndex < int(m_actors.size()));

	return m_actors[ActorIndex];
}

NxActor* GrannyPhysX::getActorByName(char const* ActorName)
{
	assert(IsValid());
	assert(ActorName);

	{for(size_t ActorIdx = 0; ActorIdx < m_actors.size(); ActorIdx++)
	{
		NxActor* Actor = m_actors[ActorIdx];
		assert(Actor);

		if (strcmp(Actor->getName(), ActorName) == 0)
			return Actor;
	}}
	
	return NULL;
}

NxActor* GrannyPhysX::getActorForBone(int const BoneIndex)
{
	assert(IsValid());
	assert(BoneIndex >= 0 && BoneIndex < m_sourceSkeleton->BoneCount);
	
	if (m_boneToActorMap[BoneIndex] != GRP_NO_MAPPING)
		return m_actors[m_boneToActorMap[BoneIndex]];
	
	return NULL;
}


#endif


NXU::NxuPhysicsCollection * grp_loadCollection(const char *fname) // loads an .XML file and returns the 'NxuPhysicsCollection'
{

  NXU::NxuPhysicsCollection *ret = 0;

	ret = NXU::loadCollection(fname,NXU::FT_XML,0);


  return ret;
}


void  grp_releaseCollection(NXU::NxuPhysicsCollection *collection)
{
	NXU::releaseCollection(collection);
}

