/***************************************************************************
*	Copyright (C) 2017 by Renaud Guezennec                             *
*   http://www.rolisteam.org/contact                   *
*                                                                         *
*   Rolisteam is free software; you can redistribute it and/or modify     *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "diceroller.h"

DiceRoller::DiceRoller()
{

}

QString DiceRoller::getDiceList() const
{
    return m_diceList;
}

QString DiceRoller::getResultStr() const
{
    return m_resultStr;
}


QString DiceRoller::getCommand() const
{
    return m_command;
}

qreal DiceRoller::getResult() const
{
    return m_result;
}

void DiceRoller::setCommand(const QString &cmd)
{
    if(m_command != cmd)
    {
        m_command = cmd;
        emit commandChanged();
    }
}

QString DiceRoller::diceToText(QList<ExportedDiceResult>& diceList)
{
    QStringList global;
    for(auto dice : diceList)
    {
        QStringList resultGlobal;
        for(int face:  dice.keys())
        {
            QStringList result;
            ListDiceResult diceResult =  dice.value(face);
            for (const HighLightDice& tmp : diceResult)
            {
                QStringList diceListStr;
                QStringList diceListChildren;
                for(int i =0; i < tmp.getResult().size(); ++i)
                {
                    qint64 dievalue = tmp.getResult()[i];
                    QString prefix("%1");

                    if(i==0)
                    {
                        diceListStr << prefix.arg(QString::number(dievalue));
                    }
                    else
                    {
                        diceListChildren << prefix.arg(QString::number(dievalue));
                    }
                }
                if(!diceListChildren.isEmpty())
                {
                    diceListStr << QString("[%1]").arg(diceListChildren.join(' '));
                }
                result << diceListStr.join(' ');
            }

            if(dice.keys().size()>1)
            {
                resultGlobal << QString(" d%2:(%1)").arg(result.join(',')).arg(face);
            }
            else
            {
                resultGlobal << result;
            }
        }
        global << resultGlobal.join(' ');
    }
    return global.join(" ; ");
}
void DiceRoller::start()
{
    if(m_diceparser.parseLine(m_command))
    {
        m_diceparser.start();
        if(m_diceparser.getErrorMap().isEmpty())
        {
            bool homogeneous;
            QList<ExportedDiceResult> list;
            m_diceparser.getLastDiceResult(list,homogeneous);
            QString diceText = diceToText(list);
            QString scalarText;
            QString str;

            qreal result = 0;
            QString resultStr;
            if(m_diceparser.hasIntegerResultNotInFirst())
            {
                auto values = m_diceparser.getLastIntegerResults();
                QStringList strLst;
                for(auto val : values )
                {
                    result += val;
                    strLst << QString::number(val);
                }
                scalarText = QString("%1").arg(strLst.join(','));
            }
            else if(!list.isEmpty())
            {
                auto values = m_diceparser.getSumOfDiceResult();
                QStringList strLst;
                for(auto val : values )
                {
                    result += val;
                    strLst << QString::number(val);
                }
                scalarText = QString("%1").arg(strLst.join(','));
            }

            if(m_diceparser.hasStringResult())
            {
                bool ok;
                QStringList allStringlist = m_diceparser.getAllStringResult(ok);
                QString stringResult = allStringlist.join(" ; ");
                stringResult.replace("%1",scalarText);
                stringResult.replace(" - %2 -",diceText.trimmed());
                str = stringResult;
            }
            else
            {
                resultStr = scalarText;
            }
            if(!m_diceparser.getComment().isEmpty())
            {
                resultStr+=m_diceparser.getComment()+"\n";
            }
            resultStr += str+"\n";
            m_resultStr = resultStr;
            m_result = result;
            m_diceList = diceText.trimmed();
            emit resultStrChanged();
            emit resultChanged();
            emit diceListChanged();
        }

    }

    if(!m_diceparser.getErrorMap().isEmpty())
    {
        auto errors = m_diceparser.getErrorMap();
        setError(errors.first());
    }


}

QString DiceRoller::getError() const
{
    return m_error;
}

void DiceRoller::setError(const QString &error)
{
    if(m_error != error)
    {
        m_error = error;
        emit errorOccurs();
    }
}

