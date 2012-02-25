/*LICENSE_START*/ 
/* 
 *  Copyright 1995-2002 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 
#include "StatisticsTest.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "FastStatistics.h"
#include "DescriptiveStatistics.h"

using namespace caret;
using namespace std;

StatisticsTest::StatisticsTest(const AString& identifier) : TestInterface(identifier)
{
}

void StatisticsTest::execute()
{
    srand(time(NULL));
    const int NUM_ELEMENTS = 1 << 20;
    float myData[NUM_ELEMENTS];
    for (int i = 0; i < NUM_ELEMENTS; ++i)
    {
        myData[i] = (rand() * 100.0f / RAND_MAX) - 50.0f;
    }
    DescriptiveStatistics myFullStats;
    myFullStats.update(myData, NUM_ELEMENTS);
    FastStatistics myFastStats(myData, NUM_ELEMENTS);
    float exacttolerance = myFullStats.getPopulationStandardDeviation() * 0.000001f;
    float approxtolerance = myFullStats.getPopulationStandardDeviation() * 0.01f;
    if (abs(myFullStats.getMean() - myFastStats.getMean()) > exacttolerance)
    {
        setFailed(AString("mismatch in mean, full: ") + AString::number(myFullStats.getMean()) + ", fast: " + AString::number(myFastStats.getMean()));
    }
    if (abs(myFullStats.getStandardDeviationSample() - myFastStats.getSampleStdDev()) > exacttolerance)
    {
        setFailed(AString("mismatch in sample stddev, full: ") + AString::number(myFullStats.getStandardDeviationSample()) + ", fast: " + AString::number(myFastStats.getSampleStdDev()));
    }
    if (abs(myFullStats.getPopulationStandardDeviation() - myFastStats.getPopulationStdDev()) > exacttolerance)
    {
        setFailed(AString("mismatch in population stddev, full: ") + AString::number(myFullStats.getPopulationStandardDeviation()) + ", fast: " + AString::number(myFastStats.getPopulationStdDev()));
    }
    if (abs(myFullStats.getMedian() - myFastStats.getApproximateMedian()) > approxtolerance)
    {
        setFailed(AString("mismatch in median, full: ") + AString::number(myFullStats.getMedian()) + ", fast: " + AString::number(myFastStats.getApproximateMedian()));
    }
    if (abs(myFullStats.getPositivePercentile(90.0f) - myFastStats.getApproxPositivePercentile(90.0f)) > approxtolerance)
    {
        setFailed(AString("mismatch in 90% positive percentile, full: ") + AString::number(myFullStats.getPositivePercentile(90.0f)) + ", fast: " + AString::number(myFastStats.getApproxPositivePercentile(90.0f)));
    }
    if (abs(myFullStats.getNegativePercentile(90.0f) - myFastStats.getApproxNegativePercentile(90.0f)) > approxtolerance)
    {
        setFailed(AString("mismatch in 90% negative percentile, full: ") + AString::number(myFullStats.getNegativePercentile(90.0f)) + ", fast: " + AString::number(myFastStats.getApproxNegativePercentile(90.0f)));
    }
}
