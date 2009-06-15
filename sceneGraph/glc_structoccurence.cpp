/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 1.1.0, packaged on March, 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_structoccurence.cpp implementation of the GLC_StructOccurence class.

#include "glc_structoccurence.h"
#include "glc_collection.h"
#include "glc_structreference.h"
#include "glc_worldhandle.h"

// Default constructor
GLC_StructOccurence::GLC_StructOccurence(GLC_WorldHandle* pWorldHandle, GLC_StructInstance* pStructInstance, GLuint shaderId)
: m_Uid(glc::GLC_GenID())
, m_pWorldHandle(pWorldHandle)
, m_pNumberOfOccurence(NULL)
, m_pStructInstance(pStructInstance)
, m_pParent(NULL)
, m_Childs()
, m_AbsoluteMatrix()
, m_HasRepresentation(pStructInstance->structReference()->hasRepresentation())
{
	// Update the number of occurences
	if (pStructInstance->hasStructOccurence())
	{
		m_pNumberOfOccurence= pStructInstance->firstOccurenceHandle()->m_pNumberOfOccurence;
		++(*m_pNumberOfOccurence);
		QList<GLC_StructOccurence*> childs= pStructInstance->firstOccurenceHandle()->m_Childs;
		const int size= childs.size();
		for (int i= 0; i < size; ++i)
		{
			addChild(childs.at(i)->clone(m_pWorldHandle));
		}
	}
	else
	{
		m_pNumberOfOccurence= new int(1);
	}

	setName(m_pStructInstance->name());

	// Inform the world Handle
	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->addOccurence(this, shaderId);
	}

	// Update Absolute matrix
	updateAbsoluteMatrix();

	// Update instance
	m_pStructInstance->structOccurenceCreated(this);
}
// Copy constructor
GLC_StructOccurence::GLC_StructOccurence(GLC_WorldHandle* pWorldHandle, const GLC_StructOccurence& structOccurence)
: m_Uid(structOccurence.m_Uid)
, m_pWorldHandle(pWorldHandle)
, m_pNumberOfOccurence(structOccurence.m_pNumberOfOccurence)
, m_pStructInstance(structOccurence.m_pStructInstance)
, m_pParent(NULL)
, m_Childs()
, m_AbsoluteMatrix(structOccurence.m_AbsoluteMatrix)
, m_HasRepresentation(structOccurence.m_HasRepresentation)
{
	// Change object id
	m_Uid= glc::GLC_GenID();
	++(*m_pNumberOfOccurence);

	// Test if structOccurence has representation and has a shader
	GLuint shaderId= 0;
	bool instanceIsSelected= false;
	if ((m_HasRepresentation) and (NULL != m_pWorldHandle) and (NULL != structOccurence.m_pWorldHandle))
	{

		if(structOccurence.m_pWorldHandle->collection()->isInAShadingGroup(structOccurence.id()))
		{
			shaderId= structOccurence.m_pWorldHandle->collection()->shadingGroup(structOccurence.id());
		}

		instanceIsSelected= structOccurence.m_pWorldHandle->collection()->instanceHandle(structOccurence.id())->isSelected();
	}

	// Inform the world Handle
	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->addOccurence(this,instanceIsSelected, shaderId);
	}

	// Update Absolute matrix
	updateAbsoluteMatrix();


	// Create childs
	const int size= structOccurence.childCount();
	for (int i= 0; i < size; ++i)
	{
		GLC_StructOccurence* pChild= structOccurence.child(i)->clone(m_pWorldHandle);
		addChild(pChild);
		pChild->updateAbsoluteMatrix();
	}

	// Update instance
	m_pStructInstance->structOccurenceCreated(this);
}

// Destructor
GLC_StructOccurence::~GLC_StructOccurence()
{
	Q_ASSERT(m_pNumberOfOccurence != NULL);
	// Remove from the GLC_WorldHandle
	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->removeOccurence(this);
	}

	// Remove Childs
	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		GLC_StructOccurence* pChild= m_Childs.at(i);
		delete pChild;
	}
	// Update number of occurence and instance
	if ((--(*m_pNumberOfOccurence)) == 0)
	{
		delete m_pStructInstance;
		delete m_pNumberOfOccurence;
	}
	else
	{
		m_pStructInstance->structOccurenceDeleted(this);
	}
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Get number of faces
unsigned int GLC_StructOccurence::numberOfFaces() const
{
	unsigned int result= 0;
	if (m_HasRepresentation)
	{
		result= structInstance()->structReference()->numberOfFaces();
	}
	else
	{
		const int size= m_Childs.size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_Childs.at(i)->numberOfFaces();
		}
	}
	return result;
}

// Get number of vertex
unsigned int GLC_StructOccurence::numberOfVertex() const
{
	unsigned int result= 0;
	if (m_HasRepresentation)
	{
		result= structInstance()->structReference()->numberOfVertex();
	}
	else
	{
		const int size= m_Childs.size();
		for (int i= 0; i < size; ++i)
		{
			result+= m_Childs.at(i)->numberOfVertex();
		}
	}
	return result;
}

// Get number of materials
unsigned int GLC_StructOccurence::numberOfMaterials() const
{
	unsigned int result= 0;
	QSet<GLC_Material*> materialSet;
	if (m_HasRepresentation)
	{
		result= structInstance()->structReference()->numberOfMaterials();
	}
	else
	{
		const int size= m_Childs.size();
		for (int i= 0; i < size; ++i)
		{
			materialSet.unite(m_Childs.at(i)->materialSet());
		}
		result= static_cast<unsigned int>(materialSet.size());
	}
	return result;
}

// Get materials List
QSet<GLC_Material*> GLC_StructOccurence::materialSet() const
{
	QSet<GLC_Material*> materialSet;
	if (m_HasRepresentation)
	{
		materialSet= structInstance()->structReference()->materialSet();
	}
	else
	{
		const int size= m_Childs.size();
		for (int i= 0; i < size; ++i)
		{
			materialSet.unite(m_Childs.at(i)->materialSet());
		}
	}
	return materialSet;
}

// Clone the occurence
GLC_StructOccurence* GLC_StructOccurence::clone(GLC_WorldHandle* pWorldHandle) const
{
	return new GLC_StructOccurence(pWorldHandle, *this);
}

// Return true if the occurence is visible
bool GLC_StructOccurence::isVisible() const
{
	bool isHidden= true;
	if (m_HasRepresentation)
	{
		isHidden= not m_pWorldHandle->collection()->instanceHandle(id())->isVisible();
	}
	else if (childCount() > 0)
	{
		const int size= childCount();
		int i= 0;
		while ((i < size) and isHidden)
		{
			isHidden= isHidden and not child(i)->isVisible();
			++i;
		}
	}
	else
	{
		qDebug() << "Leaf occurence " << id() << " " << name() << " without rep";
	}
	return not isHidden;
}

// Return the occurence Bounding Box
GLC_BoundingBox GLC_StructOccurence::boundingBox() const
{
	GLC_BoundingBox boundingBox;

	if (not isOrphan() and (NULL != m_pWorldHandle))
	{
		if (m_HasRepresentation)
		{
			Q_ASSERT(m_pWorldHandle->collection()->contains(id()));
			boundingBox= m_pWorldHandle->collection()->instanceHandle(id())->getBoundingBox();
		}
		else
		{
			if (hasChild())
			{
				QList<GLC_StructOccurence*> childrenList= children();
				const int size= childrenList.size();

				for (int i= 0; i < size; ++i)
				{
					boundingBox.combine(childrenList.at(i)->boundingBox());
				}
			}
		}
	}
	return boundingBox;
}


//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Update the absolute matrix
GLC_StructOccurence* GLC_StructOccurence::updateAbsoluteMatrix()
{
	if (NULL != m_pParent)
	{
		m_AbsoluteMatrix= m_pParent->absoluteMatrix() * m_pStructInstance->relativeMatrix();
	}
	else
	{
		m_AbsoluteMatrix= m_pStructInstance->relativeMatrix();
	}
	// If the occurence have a representation, update it.
	if (m_HasRepresentation)
	{
		m_pWorldHandle->collection()->instanceHandle(id())->setMatrix(m_AbsoluteMatrix);
	}
	return this;
}

// Update childs obsolute Matrix
GLC_StructOccurence* GLC_StructOccurence::updateChildsAbsoluteMatrix()
{
	updateAbsoluteMatrix();
	const int size= m_Childs.size();
	for (int i= 0; i < size; ++i)
	{
		m_Childs[i]->updateChildsAbsoluteMatrix();
	}
	return this;
}

// Add Child
void GLC_StructOccurence::addChild(GLC_StructOccurence* pChild)
{
	Q_ASSERT(pChild->isOrphan());
	Q_ASSERT((NULL == pChild->m_pWorldHandle) or (m_pWorldHandle == pChild->m_pWorldHandle));

	//qDebug() << "Add Child " << pChild->name() << "id=" << pChild->id() << " to " << name() << " id=" << id();
	// Add the child to the list of child
	m_Childs.append(pChild);
	pChild->m_pParent= this;
	if (NULL == pChild->m_pWorldHandle)
	{
		m_pWorldHandle->addOccurence(pChild);
	}
	pChild->updateAbsoluteMatrix();
}

// Add Child instance (the occurence is created)
void GLC_StructOccurence::addChild(GLC_StructInstance* pInstance)
{
	GLC_StructOccurence* pOccurence;
	if (not pInstance->hasStructOccurence())
	{
		pOccurence= new GLC_StructOccurence(m_pWorldHandle, pInstance);
	}
	else
	{
		pOccurence= pInstance->firstOccurenceHandle()->clone(m_pWorldHandle);
	}

	addChild(pOccurence);
}

// Add Children
void GLC_StructOccurence::addChildren(const QList<GLC_StructOccurence*>& children)
{
	const int size= children.size();
	for (int i= 0; i < size; ++i)
	{
		Q_ASSERT(children.at(i)->isOrphan());
		// Add the child to the list of child
		m_Childs.append(children.at(i));
		children.at(i)->m_pParent= this;
	}
}

// make the occurence orphan
void GLC_StructOccurence::makeOrphan()
{
	Q_ASSERT(not isOrphan());
	m_pParent->removeChild(this);
}

// Remove the specified child
bool GLC_StructOccurence::removeChild(GLC_StructOccurence* pChild)
{
	pChild->m_pParent= NULL;
	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->removeOccurence(pChild);
		m_pWorldHandle= NULL;
	}
	return m_Childs.removeOne(pChild);
}

// Reverse Normals of this Occurence and childs
void GLC_StructOccurence::reverseNormals()
{
	if (m_HasRepresentation)
	{
		m_pWorldHandle->collection()->instanceHandle(id())->reverseGeometriesNormals();
	}
}

// Check the presence of representation
void GLC_StructOccurence::checkForRepresentation()
{
	if (NULL != m_pStructInstance)
	{
		GLC_StructReference* pRef= m_pStructInstance->structReference();
		if (NULL != pRef)
		{
			if (pRef->hasRepresentation())
			{
				GLC_Instance representation(pRef->instanceRepresentation());
				representation.setName(name());
				// Force instance representation id
				representation.setId(id());
				m_pWorldHandle->collection()->add(representation);
			}
			m_HasRepresentation= true;
		}
	}
}

// Set the occurence world Handle
void GLC_StructOccurence::setWorldHandle(GLC_WorldHandle* pWorldHandle)
{
	// Check if world handles are equal
	if (m_pWorldHandle == pWorldHandle) return;

	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->removeOccurence(this);
	}

	m_pWorldHandle= pWorldHandle;

	if (NULL != m_pWorldHandle)
	{
		m_pWorldHandle->addOccurence(this);
		const int size= m_Childs.size();
		for (int i= 0; i < size; ++i)
		{
			m_Childs[i]->setWorldHandle(m_pWorldHandle);
		}
	}
}
