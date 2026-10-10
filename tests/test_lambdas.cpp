#include <gtest/gtest.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <optional>
#include <iostream>


std::vector<double> measures = {1.2, 3.5, 3.8, 4.2, 4.8, 2.1, 5.7, 1.1};
std::vector<double> sortedMeasureOriginal = {5.7, 4.8, 4.2, 3.8, 3.5, 2.1, 1.2, 1.1};
std::vector<double> measuresPercent(measures.size());
std::vector<double> measuresPercenResult = {20.0, 58.3333, 63.3333, 70.0, 80.0, 35.0, 95.0, 18.3333};
int countMeasures(const std::vector<double>& measures, std::function<bool(double)> predicate)
{
    return std::count_if( measures.begin(), measures.end(), predicate);
};

TEST(PracticeWithLambda, CountValuesUnderThreePointSix){
    int nombre = std::count_if(
        measures.begin(),
        measures.end(),
        [] (double measure){
            return measure < 3.6;
        }
    );
    EXPECT_EQ(nombre, 4);
    std::cout << "Nombre de valeurs passant le seuil: " << nombre << std::endl;
}

TEST(PracticeWithLambda, FirstValueAboveThreshold){
    auto it = std::find_if(
        measures.begin(),
        measures.end(),
        [] (double measure){
            return measure > 3.6;
        }
    );
    ASSERT_NE(it, measures.end());
    EXPECT_EQ(*it, 3.8);
}

TEST(PracticeWithLambda, indexFirstValueAboveThreshold){
    auto it = std::find_if(
        measures.begin(),
        measures.end(),
        [] (double measure){
            return measure > 3.6;
        }
    );
    ASSERT_NE(it, measures.end());
    auto distance = std::distance(measures.begin(), it);
    std::cout << "Valeur trouvee : " << *it << '\n';
    std::cout << "Position : " << distance << '\n';
    EXPECT_EQ(distance, 2);
    EXPECT_EQ(*it, 3.8);
}

TEST(PracticeWithLambda, sortValues){
    auto sortedMeasures = measures;
    std::sort(
        sortedMeasures.begin(),
        sortedMeasures.end(),
        [] (double measure, double measureTwo){
            return measure > measureTwo;
        }
    );
    EXPECT_EQ(sortedMeasures, sortedMeasureOriginal);
}

TEST(PracticeWithLambda, transformToPercentage){
    double coeff = 100.0/6.0;
    std::transform(
        measures.begin(),
        measures.end(),
        measuresPercent.begin(),
        [coeff](double measure){
            return measure * coeff;
        }
    );
    for (std::size_t i = 0; i < measuresPercent.size(); ++i) {
        EXPECT_NEAR(measuresPercent[i], measuresPercenResult[i], 0.0001);
    }
}

TEST(PracticeWithLambda, CountValuesOverThreshold){
    double threshold = 4.7;
    int nombre = std::count_if(
        measures.begin(),
        measures.end(),
        [threshold] (double measure){
            return measure > threshold;
        }
    );
    EXPECT_EQ(nombre, 2);
}

TEST(PracticeWithLambda, LambdaBehviorCopyMutable){
    int threshold = 1;
    auto testFunction = [threshold]() mutable {
        threshold++;
        return threshold;
    };
    threshold = 10;
    auto result = testFunction();
    EXPECT_EQ(result, 2);
    EXPECT_EQ(threshold, 10);
    result = testFunction();
    EXPECT_EQ(result, 3);
}

TEST(PracticeWithLambda, LambdaBehaviorCopy){
    int threshold = 1;
    auto testFunction = [threshold]() {
        return threshold;
    };
    threshold = 10;
    auto result = testFunction();
    EXPECT_EQ(result, 1);
}

TEST(PracticeWithLambda, LambdaBehaviorReference){
    int threshold = 1;
    auto testFunction = [&threshold]() {
        return threshold;
    };
    threshold = 10;
    auto result = testFunction();
    EXPECT_EQ(result, 10);
}

TEST(PracticeWithLambda, CountValuesOverThresholdWithFunction){
    std::function<bool(double)> isUnderThreshold =
    [](double measure) {
        return measure < 3.6;
    };

    int nombre = std::count_if(
        measures.begin(),
        measures.end(),
        isUnderThreshold
    );
    EXPECT_EQ(nombre, 4);

    isUnderThreshold = [](double measure) {
        return measure < 4.6;
    };

    nombre = std::count_if(
        measures.begin(),
        measures.end(),
        isUnderThreshold
    );

    EXPECT_EQ(nombre, 6);
}

TEST(PracticeWithLambda, FunctionWithTwoLambdas){
    int countSmall = countMeasures(
        measures,
        [](double measure) {
            return measure < 3.6;
        }
    );

    int countLarge = countMeasures(
        measures,
        [](double measure) {
            return measure > 4.0;
        }
    );

    EXPECT_EQ(countSmall, 4);
    EXPECT_EQ(countLarge, 3);
}