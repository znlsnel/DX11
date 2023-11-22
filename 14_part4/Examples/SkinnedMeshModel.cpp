#include "SkinnedMeshModel.h"
#include "Character.h"


void hlab::SkinnedMeshModel::UpdatePose(ComPtr<ID3D11DeviceContext> &context,  float dt) {

        vector<Matrix> temp(m_boneTransforms.m_cpu.size());

        //currAnim->GetCurrFramePos(m_boneTransforms.m_cpu, dt);

        if (prevAnim == nullptr) 
        {
             currAnim->GetCurrFramePos(m_boneTransforms.m_cpu, dt);

             //float rt = currAnim->frame / float(currAnim->endFrame);
             //if (rt >= 0.99f)
             //    currAnim->frame = currAnim->blendFrame;
             //else if (rt > 0.5f) 
             //{
             //    currAnim->GetCurrFramePos(temp, dt, true);
             //    // 0.6 ~ 1 -> 0 ~ 1
             //    cout << "frame : " << currAnim->frame << endl;
             //    cout << "Endframe : " << currAnim->endFrame << endl;
             //    cout << "rt : " << rt << endl;

             //    for (int i = 0; i < m_boneTransforms.m_cpu.size(); i++) 
             //    {
             //        m_boneTransforms.m_cpu[i] = Matrix::Lerp(
             //            m_boneTransforms.m_cpu[i], temp[i],
             //            std::clamp((rt - 0.5f) / 0.5f, 0.0f, 1.0f));
             //    }   
             //} 
             //else 
             //{
             //    currAnim->blendFrame = 0.0f;
             //}

        } 
        else{
                blendingTime += dt;

                if (blendingTime < currAnim->blendTime) {
                    currAnim->GetCurrFramePos(temp, dt);

                        for (int i = 0; i < m_boneTransforms.m_cpu.size();
                             i++) {
                            m_boneTransforms.m_cpu[i] =
                                Matrix::Lerp(m_boneTransforms.m_cpu[i], temp[i],
                                std::clamp(prevAnim->lerpValue, 0.0f, 1.0f));
                        }
                    }
                        prevAnim->lerpValue += dt / currAnim->blendTime;
                    



                if (blendingTime >= currAnim->blendTime) {
                        prevAnim->lerpValue = 0.0f;
                        prevAnim = nullptr;
                }

                //else {
                //         prevAnim->GetCurrFramePos(m_boneTransforms.m_cpu, dt );
                //
                //}
                // idle -> walk -> idle
                if (prevAnim != nullptr && (int) prevAnim->frame == prevAnim->endFrame) {
                         prevAnim = nullptr;

                }
                 
        }


        m_boneTransforms.Upload(context);

}

void hlab::SkinnedMeshModel::AnimationInit() {
        idle =
            Animation(0, m_aniData.clips[0].keys[0].size(), &m_aniData);
        walk =
            Animation(1, m_aniData.clips[1].keys[0].size(), &m_aniData);
        walk_backward =
            Animation(2, m_aniData.clips[2].keys[0].size(), &m_aniData);

        run = Animation(3, m_aniData.clips[3].keys[0].size(), &m_aniData);

        run_backward =
            Animation(4, m_aniData.clips[4].keys[0].size(), &m_aniData);
        jumping_Up = Animation(5, m_aniData.clips[5].keys[0].size(), &m_aniData);
        jumping_Down =
            Animation(6, m_aniData.clips[6].keys[0].size(), &m_aniData);
        jumping_Falling =
            Animation(6, m_aniData.clips[6].keys[0].size(), &m_aniData);

}

void hlab::SkinnedMeshModel::ChangeAnimation(Animation *anim) 
{
        if (currAnim == anim)
                return;

        prevAnim = currAnim;
        currAnim = anim;
        currAnim->InitFrame();

        blendingTime = 0.0f;

}

void hlab::SkinnedMeshModel::ChangeAnimation(EActorState state, bool forward) {


        Animation *anim = &idle;
        switch (state) {
        case EActorState::idle:
                break;
        case EActorState::walk: {
                if (forward)
                         anim = &walk;
                else
                         anim = &walk_backward;
        }
                break;
        case EActorState::run: {
                if (forward)
                         anim = &run;
                else
                         anim = &run_backward;
        }
                break;
        }
        ChangeAnimation(anim);
}

void hlab::SkinnedMeshModel::UpdateAnimation(
    ComPtr<ID3D11DeviceContext> &context, int clipId, float frame) {

        int currFrame = (int)frame;
        int nextFrame = std::min(currFrame + 1,
                                 (int)m_aniData.clips[clipId].keys[0].size());
        float lerpValue = (float)frame - currFrame;

        vector<Matrix> tempFrameMatrix(m_boneTransforms.m_cpu.size());

        m_aniData.Update(clipId, currFrame);
        for (int i = 0; i < tempFrameMatrix.size(); i++) {
                tempFrameMatrix[i] =
                    m_aniData.Get(clipId, i, currFrame).Transpose();
        }

        m_aniData.Update(clipId, nextFrame);
        for (int i = 0; i < m_boneTransforms.m_cpu.size(); i++) {

                m_boneTransforms.m_cpu[i] = Matrix::Lerp(
                    tempFrameMatrix[i],
                    m_aniData.Get(clipId, i, currFrame).Transpose(), lerpValue);
                // m_boneTransforms.m_cpu[i] = currFrameMatrix[i];
        }

        m_boneTransforms.Upload(context);
}

