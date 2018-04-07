/***************************************************************************
* Copyright (C) 2014 by Renaud Guezennec                                   *
* http://www.rolisteam.org/contact                      *
*                                                                          *
*  This file is part of DiceParser                                         *
*                                                                          *
* DiceParser is free software; you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by     *
* the Free Software Foundation; either version 2 of the License, or        *
* (at your option) any later version.                                      *
*                                                                          *
* This program is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
* GNU General Public License for more details.                             *
*                                                                          *
* You should have received a copy of the GNU General Public License        *
* along with this program; if not, write to the                            *
* Free Software Foundation, Inc.,                                          *
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.                 *
***************************************************************************/
#include "groupnode.h"
#include "result/diceresult.h"
//-------------------------------
int DieGroup::getSum() const
{
    int sum =0;
    for(int i : *this)
    {
        sum += i;
    }
    return sum;
}

void DieGroup::removeValue(DieGroup i)
{
    for(auto x : i )
    {
        removeOne(x);
    }
}

int DieGroup::getLost() const
{
    return getSum()-m_exceptedValue;
}

qint64 DieGroup::getExceptedValue() const
{
    return m_exceptedValue;
}

void DieGroup::setExceptedValue(qint64 exceptedValue)
{
    m_exceptedValue = exceptedValue;
}




//---------------------
GroupNode::GroupNode()
    : m_scalarResult(new ScalarResult())
{
    m_result = m_scalarResult;
}
void GroupNode::run(ExecutionNode* previous)
{
    m_previousNode = previous;
    if(nullptr != previous)
    {
        m_result->setPrevious(previous->getResult());
        Result* tmpResult = previous->getResult();
        if(nullptr != tmpResult)
        {
            DiceResult* dice = dynamic_cast<DiceResult*>(tmpResult);
            if(nullptr != dice)
            {
                auto list = dice->getResultList();
                DieGroup allResult;
                for(Die* die : dice->getResultList())
                {
                    allResult << die->getListValue();
                }
                std::sort(allResult.begin(),allResult.end(), std::greater<qint64>());
                if(allResult.getSum() > m_groupValue)
                {
                    auto const die =getGroup(allResult);
                    m_scalarResult->setValue(die.size());
                }
                else
                {
                    m_scalarResult->setValue(0);
                }
            }
        }
    }
    if(nullptr!=m_nextNode)
    {
        m_nextNode->run(this);
    }
}

QString GroupNode::toString(bool withLabel) const
{
    if(withLabel)
    {
        return QString("%1 [label=\"SplitNode Node\"]").arg(m_id);
    }
    else
    {
        return m_id;
    }
}
qint64 GroupNode::getPriority() const
{
    qint64 priority=0;
    if(nullptr!=m_nextNode)
    {
        priority = m_nextNode->getPriority();
    }
    return priority;
}
ExecutionNode* GroupNode::getCopy() const
{
    GroupNode* node = new GroupNode();
    if(nullptr!=m_nextNode)
    {
        node->setNextNode(m_nextNode->getCopy());
    }
    return node;

}

int GroupNode::getGroupValue() const
{
    return m_groupValue;
}

void GroupNode::setGroupValue(qint64 groupValue)
{
    m_groupValue = groupValue;
}

bool GroupNode::composeWithPrevious(DieGroup previous, qint64 first, qint64 current, DieGroup& addValue)
{
    if(previous.getSum()+first+current == m_groupValue)
    {
        addValue.append(previous);
        addValue.append(first);
        addValue.append(current);
        return true;
    }

    if(previous.isEmpty())
        return false;

    int maxComboLength = previous.size();
    bool hasReachMax = false;

    QList<DieGroup> possibleUnion;
    for(auto va : previous)
    {
        DieGroup dieG;
        dieG.append(va);
        possibleUnion.append(dieG);
    }
    while(!hasReachMax)
    {
        auto tmpValues = previous;
        QList<DieGroup> possibleTmp;
        for(auto& diaG : possibleUnion)
        {
            tmpValues.removeValue(diaG);
            for(auto& value : tmpValues)
            {
                DieGroup dia;
                dia.append(diaG);
                dia.append(value);
                possibleTmp.append(dia);
                if(dia.size() >= maxComboLength-1)
                    hasReachMax = true;
            }
        }
        if(possibleTmp.isEmpty())
            hasReachMax = true;
        possibleUnion.append(possibleTmp);

    }
    std::sort(possibleUnion.begin(),possibleUnion.end(),[=](const DieGroup& a, const DieGroup& b){
        return a.getLost() > b.getLost();
    });
    bool found = false;
    for(int i = 0;(!found && i < possibleUnion.size()); ++i)
    {
        auto& value = possibleUnion.at(i);
        if(value.getSum() + current + first >= m_groupValue)
        {
            addValue << value << current << first;
            found = true;
        }
    }
    return found;
}

#include <list>
#include <algorithm>

QList<DieGroup> GroupNode::getGroup(DieGroup values)
{
    auto first = values.takeFirst();

    if(values.isEmpty())
        return QList<DieGroup>();

    QList<DieGroup> result;
    QMap<qint64,DieGroup> loseMap;
    if(first == m_groupValue)
    {
        DieGroup group;
        group << first ;
        loseMap[0] = group;
    }
    else
    {
	auto x = std::list<qint64>();
	std::copy(std::begin(values), std::end(values), std::begin(x));
        auto it = x.crbegin();
        bool foundPerfect = false;
        qint64 cumuledValue = 0;
        DieGroup previousValue;
        while((x.rend() != it) && !foundPerfect)
        {
            if(first+*it == m_groupValue)
            {
                foundPerfect = true;
                DieGroup group;
                group << first << *it;
                loseMap[0] = group;
            }
            else if(first+*it > m_groupValue)
            {
                DieGroup group;
                group << first << *it;
                loseMap[first+*it-m_groupValue]=group;
            }
            else if(first+*it+cumuledValue == m_groupValue)
            {
                DieGroup group;
                group << first << *it << previousValue;
                foundPerfect = true;
                loseMap[0] = group;
            }
            else if(first+*it+cumuledValue > m_groupValue)
            {
                DieGroup group;
                group.setExceptedValue(m_groupValue);
                composeWithPrevious(previousValue,first, *it, group);
                loseMap[group.getLost()]= group;
            }
            previousValue << *it;
            cumuledValue += *it;
            ++it;
        }
    }
    DieGroup die = loseMap.first();
    result.append(die);
    DieGroup valueToRemove = die;
    valueToRemove.removeFirst();
    values.removeValue(valueToRemove);

    if(values.getSum() >= m_groupValue)
    {
        result.append(getGroup(values));
    }
    return result;

}


