#include "AnimationBank.h"
#include <map>
AnimationBank::AnimationBank() {

}
AnimationBank::~AnimationBank() {

}
AnimationBank&  AnimationBank::GetInstance() {
	static AnimationBank m_Bank;
	return m_Bank;
}
