/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          https://www.mrpt.org/                            |
   |                                                                           |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file        |
   | See: https://www.mrpt.org/Authors - All rights reserved.                  |
   | Released under BSD License. See details in https://www.mrpt.org/License   |
   +---------------------------------------------------------------------------+
   */
#include "CRobotPose.h"

#include <mrpt/opengl/stock_objects.h>

CRobotPose::CRobotPose(size_t id) : CSetOfObjects(), m_id(id)
{
	m_currentObj = mrpt::opengl::stock_objects::CornerXYZSimple();
	insert(m_currentObj);
}

size_t CRobotPose::getId() const { return m_id; }
void CRobotPose::setSelected(bool is)
{
	removeObject(m_currentObj);

	if (is) m_currentObj = mrpt::opengl::stock_objects::CornerXYZEye();
	else
		m_currentObj = mrpt::opengl::stock_objects::CornerXYZSimple();

	insert(m_currentObj);
}
