#ifndef LI_CrossSection_H
#define LI_CrossSection_H

#include <map>
#include <set>
#include <array>
#include <string>

#include <photospline/splinetable.h>

#include "LeptonInjector/Particle.h"
#include "LeptonInjector/Random.h"

namespace LeptonInjector {

struct InteractionSignature {
    LeptonInjector::Particle::ParticleType primary_type;
    LeptonInjector::Particle::ParticleType target_type;
    std::vector<LeptonInjector::Particle::ParticleType> secondary_types;
};

struct InteractionRecord {
    InteractionSignature signature;
    double primary_mass = 0;
    double target_mass = 0;
    std::array<double, 4> primary_momentum = {0, 0, 0, 0};
    std::array<double, 4> target_momentum = {0, 0, 0, 0};
    std::array<double, 3> interaction_vertex = {0, 0, 0};
    std::vector<std::array<double, 4>> secondary_momenta;
    std::vector<double> interaction_parameters;
};

class CrossSection {
private:
    //
public:
    CrossSection() {};
    virtual double TotalCrossSection(InteractionRecord const &) const = 0;
    virtual double DifferentialCrossSection(InteractionRecord const &) const = 0;
    virtual void SampleFinalState(InteractionRecord &, std::shared_ptr<LeptonInjector::LI_random>) const = 0;

    virtual std::vector<Particle::ParticleType> GetPossibleTargets() const = 0;
    virtual std::vector<Particle::ParticleType> GetPossibleTargetsFromPrimary(Particle::ParticleType primary_type) const = 0;
    virtual std::vector<Particle::ParticleType> GetPossiblePrimaries() const = 0;
    virtual std::vector<InteractionSignature> GetPossibleSignatures() const = 0;

    virtual std::vector<InteractionSignature> GetPossibleSignaturesFromParents(Particle::ParticleType primary_type, Particle::ParticleType target_type) const = 0;
};

class CrossSectionCollection {
private:
    Particle::ParticleType primary_type;
    std::vector<std::shared_ptr<CrossSection>> cross_sections;
    std::map<Particle::ParticleType, std::vector<std::shared_ptr<CrossSection>>> cross_sections_by_target;
    std::vector<Particle::ParticleType> target_types;
    void InitializeTargetTypes();
public:
    CrossSectionCollection(Particle::ParticleType primary_type, std::vector<std::shared_ptr<CrossSection>> cross_sections);
    std::vector<std::shared_ptr<CrossSection>> GetCrossSections() const {return cross_sections;};
    std::vector<std::shared_ptr<CrossSection>> GetCrossSectionsForTarget(Particle::ParticleType p) const;
    std::map<Particle::ParticleType, std::vector<std::shared_ptr<CrossSection>>> GetCrossSectionsByTarget() const {
        return cross_sections_by_target;
    };
    std::vector<Particle::ParticleType> TargetTypes() const {
        return target_types;
    };
};

class DISFromSpline : public CrossSection {
private:
    photospline::splinetable<> differential_cross_section_;
    photospline::splinetable<> total_cross_section_;

    std::vector<InteractionSignature> signatures_;
    std::set<LeptonInjector::Particle::ParticleType> primary_types_;
    std::set<LeptonInjector::Particle::ParticleType> target_types_;
    std::map<LeptonInjector::Particle::ParticleType, std::vector<LeptonInjector::Particle::ParticleType>> targets_by_primary_types_;
    std::map<std::pair<LeptonInjector::Particle::ParticleType, LeptonInjector::Particle::ParticleType>, std::vector<InteractionSignature>> signatures_by_parent_types_;

    double minimum_Q2_;
    double target_mass_;
    int interaction_type_;

public:
    DISFromSpline(std::string differential_filename, std::string total_filename, int interaction, double target_mass, double minumum_Q2, std::set<LeptonInjector::Particle::ParticleType> primary_types, std::set<LeptonInjector::Particle::ParticleType> target_types);
    DISFromSpline(std::string differential_filename, std::string total_filename, std::set<LeptonInjector::Particle::ParticleType> primary_types, std::set<LeptonInjector::Particle::ParticleType> target_types);
    DISFromSpline(std::string differential_filename, std::string total_filename, int interaction, double target_mass, double minumum_Q2, std::vector<LeptonInjector::Particle::ParticleType> primary_types, std::vector<LeptonInjector::Particle::ParticleType> target_types);
    DISFromSpline(std::string differential_filename, std::string total_filename, std::vector<LeptonInjector::Particle::ParticleType> primary_types, std::vector<LeptonInjector::Particle::ParticleType> target_types);

    double TotalCrossSection(InteractionRecord const &) const;
    double TotalCrossSection(LeptonInjector::Particle::ParticleType primary, double energy) const;
    double DifferentialCrossSection(InteractionRecord const &) const;
    double DifferentialCrossSection(double energy, double x, double y, double secondary_lepton_mass) const;
    void SampleFinalState(InteractionRecord &, std::shared_ptr<LeptonInjector::LI_random> random) const;

    std::vector<Particle::ParticleType> GetPossibleTargets() const;
    std::vector<Particle::ParticleType> GetPossibleTargetsFromPrimary(Particle::ParticleType primary_type) const;
    std::vector<Particle::ParticleType> GetPossiblePrimaries() const;
    std::vector<InteractionSignature> GetPossibleSignatures() const;
    std::vector<InteractionSignature> GetPossibleSignaturesFromParents(Particle::ParticleType primary_type, Particle::ParticleType target_type) const;

    void LoadFromFile(std::string differential_filename, std::string total_filename);

    double GetMinimumQ2() const {return minimum_Q2_;};
    double GetTargetMass() const {return target_mass_;};
    int GetInteractionType() const {return interaction_type_;};

private:
    void ReadParamsFromSplineTable();
    void InitializeSignatures();
};

template<typename T>
struct TableData1D {
    std::vector<T> x;
    std::vector<T> f;
};

template<typename T>
struct TableData2D {
    std::vector<T> x;
    std::vector<T> y;
    std::vector<T> f;
};

template<typename T>
struct IndexFinderIrregular {
    std::vector<T> data;
    std::vector<T> diff;
    T low;
    T high;
    T range;

    IndexFinderIrregular() {};
    IndexFinderIrregular(std::set<T> x): data(x.begin(), x.end()) {
        std::sort(data.begin(), data.end());
        low = data.front();
        high = data.back();
        range = high - low;
        diff.reserve(data.size() - 1);
        for(unsigned int i=1; i<data.size(); ++i) {
            diff[i-1] = data[i] - data[i-1];
        }
    };

    std::tuple<unsigned int, T, T, T> operator()(T const & x) const {
        // Lower bound returns pointer to element that is greater than or equal to x
        // i.e. x \in (a,b] --> pointer to b, x \in (b,c] --> pointer to c
        // begin is the first element
        // distance(begin, pointer to y) --> y
        // therefore this function returns the index of the lower bin edge
        unsigned int index = std::distance(data.begin(), std::lower_bound(data.begin(), data.end(), x)) - 1;
        return std::tuple<unsigned int, T, T, T>(index, x, data[index], diff[index]);
    }
};

template<typename DataType>
struct alt_floor {
    DataType operator()(DataType const & x) {
        return std::floor(x);
    }
};

template<typename T>
struct IndexFinderRegular {
    T low;
    T high;
    T range;
    unsigned int n_points;
    T delta;

    IndexFinderRegular() {};
    IndexFinderRegular(std::set<T> x) {
        std::vector<T> points(x.begin(), x.end());
        std::sort(points.begin(), points.end());
        n_points = points.size();
        low = points.front();
        high = points.back();
        range = high - low;
        delta = range / (n_points - 1);
    };

    std::tuple<unsigned int, T, T, T> operator()(T const & x) const {
        int i = (int)alt_floor<T>()((x - low) / range * (n_points - 1));
        T lower_edge = low + i * delta;
        return std::tuple<unsigned int, T, T, T>(i, x, lower_edge, delta);
    }
};


template<typename T>
struct Indexer1D {
private:
    T min_x;
    T max_x;
    T range;
    unsigned int n_points;

    std::vector<T> points;

    bool is_log = true;
    bool is_regular = false;

    IndexFinderRegular<T> regular_index;
    IndexFinderIrregular<T> irregular_index;

public:

    Indexer1D() {};
    Indexer1D(TableData1D<T> & table) {
        AddTable(table);
    };

    static
    T MaxDist(std::vector<T> x, T avg_diff) {
        std::vector<T> dist(x.size() - 1);
        for(unsigned int i=1; i<x.size(); ++i) {
            dist[i-1] = std::abs(std::abs(x[i] - x[i-1]) - avg_diff);
            if(std::isinf(dist[i-1])) {
                return std::numeric_limits<T>::infinity();
            }
        }
        return *std::max_element(dist.begin(), dist.end());
    };

	void AddTable(TableData1D<T> & table) {
        is_regular = false;
        std::set<T> x_set(table.x.begin(), table.x.end());
        std::vector<T> x(x_set.begin(), x_set.end());
        std::sort(x.begin(), x.end());
        n_points = x.size();
        assert(n_points >= 2);

        std::vector<T> log_x(x.begin(), x.end());
        std::transform(log_x.begin(), log_x.end(), log_x.begin(), [](T t)->T{return log(t);});
        std::set<T> log_x_set(log_x.begin(), log_x.end());

        regular_index = IndexFinderRegular<T>(log_x_set);
        T relative_max_dist;

        T log_relative_max_dist = MaxDist(log_x, regular_index.delta) / regular_index.delta;

        if(log_relative_max_dist < 1e-4 and not std::isinf(regular_index.delta)) {
            is_regular = true;
            is_log = true;
        }

        if(not is_regular) {
            regular_index = IndexFinderRegular<T>(x_set);
            relative_max_dist = MaxDist(x, regular_index.delta) / regular_index.delta;
            if(relative_max_dist < 1e-4 and not std::isinf(regular_index.delta)) {
                is_regular = true;
                is_log = false;
            }
        }

        if(not is_regular) {
            is_log = log_relative_max_dist < relative_max_dist;
            if(is_log) {
                irregular_index = IndexFinderIrregular<T>(log_x_set);
            } else {
                irregular_index = IndexFinderIrregular<T>(x_set);
            }
        }

        if(is_log) {
            points = std::vector<T>(log_x_set.begin(), log_x_set.end());
        } else {
            points = std::vector<T>(x.begin(), x.end());
        }

        if(is_regular) {
            min_x = regular_index.low;
            max_x = regular_index.high;
            range = regular_index.range;
            irregular_index.data.clear();
        } else {
            min_x = irregular_index.low;
            max_x = irregular_index.high;
            range = irregular_index.range;
        }
        if(is_log) {
            min_x = exp(min_x);
            max_x = exp(max_x);
            range = max_x - min_x;
        }
    }

    std::tuple<unsigned int, T, T, T> operator()(T const & x) const {
        T val = x;

        if(is_log) {
            val = log(val);
        }

        if(is_regular) {
            return regular_index(val);
        } else {
            return irregular_index(val);
        }
    }

    T Min() const {
        return min_x;
    }
    T Range() const {
        return range;
    }
    T Max() const {
        return max_x;
    }
    bool IsRegular() const {
        return is_regular;
    }
    bool IsLog() const {
        return is_log;
    }
    unsigned int NPoints() const {
        return n_points;
    }
};

template<typename T>
struct Interpolator1D {
private:
    Indexer1D<T> indexer;
	std::map<unsigned int, T> function;
    bool is_log = false;
public:

    Interpolator1D() {};
    Interpolator1D(TableData1D<T> & table) {
        AddTable(table);
    };

	void AddTable(TableData1D<T> & table) {
        std::set<T> x(table.x.begin(), table.x.end());
        std::map<T, unsigned int> xmap;
        for(unsigned int n = 0; auto i : x) {
            xmap[i] = n;
            ++n;
        }

        assert(table.x.size() >= 2);
        assert(table.f.size() >= 2);
        indexer = Indexer1D<T>(table);

        is_log = indexer.IsLog();

        std::vector<T> function_values(table.f.begin(), table.f.end());
        if(is_log) {
            std::transform(function_values.begin(), function_values.end(), function_values.begin(), [](T t)->T{return log(t);});
        }

        for(unsigned int i=0; i<table.x.size(); ++i) {
            function[
                        xmap[table.x[i]]
            ] = function_values[i];
        }
    }

    T operator()(T const & x) const {

        std::tuple<unsigned int, T, T, T> index_result = indexer(x);
        unsigned int index = std::get<0>(index_result);
        T val = std::get<1>(index_result);
        T xa = std::get<2>(index_result);
        T delta = std::get<3>(index_result);

        if(index < 0) {
            index = 0;
        }
        else if(index >= indexer.NPoints() - 1) {
            index = indexer.NPoints() - 2;
        }

        T fa = function.at(index);
        T fb = function.at(index+1);
        T result = fa + (fb - fa) * (val - xa) / delta;

        if(is_log) {
            result = exp(result);
        }

        return result;
    }

    T MinX() const {
        return indexer.Min();
    }

    T RangeX() const {
        return indexer.Range();
    }

    T MaxX() const {
        return indexer.Max();
    }

    bool IsLog() const {
        return is_log;
    }
};


template<typename T>
struct Interpolator2D {
private:
    Indexer1D<T> indexer_x;
    Indexer1D<T> indexer_y;
	std::map<std::pair<unsigned int, unsigned int>, T> function;
    bool is_log = false;
public:

    Interpolator2D() {};
    Interpolator2D(TableData2D<T> & table) {
        AddTable(table);
    };

	void AddTable(TableData2D<T> & table) {
        std::set<T> x(table.x.begin(), table.x.end());
        std::set<T> y(table.y.begin(), table.y.end());
        std::map<T, unsigned int> xmap;
        std::map<T, unsigned int> ymap;
        for(unsigned int n = 0; auto i : x) {
            xmap[i] = n;
            ++n;
        }
        for(unsigned int n = 0; auto i : y) {
            ymap[i] = n;
            ++n;
        }

        TableData1D<T> x_data;
        TableData1D<T> y_data;

        assert(table.x.size() >= 2);
        assert(table.y.size() >= 2);
        assert(table.f.size() >= 2);

        x_data.x = table.x;
        x_data.f = table.f;
        y_data.x = table.y;
        y_data.f = table.f;

        assert(x_data.x.size() >= 2);
        assert(x_data.f.size() >= 2);
        assert(y_data.x.size() >= 2);
        assert(y_data.f.size() >= 2);

        indexer_x = Indexer1D<T>(x_data);
        indexer_y = Indexer1D<T>(y_data);

        is_log = indexer_x.IsLog() or indexer_y.IsLog();

        std::vector<T> function_values(table.f.begin(), table.f.end());
        if(is_log) {
            std::transform(function_values.begin(), function_values.end(), function_values.begin(), [](T t)->T{return log(t);});
        }

        for(unsigned int i=0; i<table.x.size(); ++i) {
            function[
                std::pair<unsigned int, unsigned int>(
                        xmap[table.x[i]],
                        ymap[table.y[i]]
                        )
            ] = function_values[i];
        }
    }

    T operator()(T const & x, T const & y) const {

        std::tuple<unsigned int, T, T, T> index_result_x = indexer_x(x);
        unsigned int index_x = std::get<0>(index_result_x);
        T val_x = std::get<1>(index_result_x);
        T xa = std::get<2>(index_result_x);
        T delta_x = std::get<3>(index_result_x);

        std::tuple<unsigned int, T, T, T> index_result_y = indexer_y(x);
        unsigned int index_y = std::get<0>(index_result_y);
        T val_y = std::get<1>(index_result_y);
        T ya = std::get<2>(index_result_y);
        T delta_y = std::get<3>(index_result_y);

        if(index_x < 0) {
            index_x = 0;
        }
        else if(index_x >= indexer_x.NPoints() - 1) {
            index_x = indexer_x.NPoints() - 2;
        }

        if(index_y < 0) {
            index_y = 0;
        }
        else if(index_y >= indexer_y.NPoints() - 1) {
            index_y = indexer_y.NPoints() - 2;
        }

        T da_x = val_x - xa;
        T db_x = delta_x - da_x;
        T da_y = val_y - ya;
        T db_y = delta_y - da_y;

        T faa = function.at(std::pair<unsigned int, unsigned int>(index_x, index_y));
        T fab = function.at(std::pair<unsigned int, unsigned int>(index_x, index_y+1));
        T fba = function.at(std::pair<unsigned int, unsigned int>(index_x+1, index_y));
        T fbb = function.at(std::pair<unsigned int, unsigned int>(index_x+1, index_y+1));

        T result = (
                db_x * db_y * faa +
                db_x * da_y * fab +
                da_x * db_y * fba +
                da_x * da_y * fbb
                ) / (delta_x * delta_y);

        if(is_log) {
            result = exp(result);
        }

        return result;
    }

    T MinX() const {
        return indexer_x.Min();
    }

    T RangeX() const {
        return indexer_x.Range();
    }

    T MaxX() const {
        return indexer_x.Max();
    }

    T MinY() const {
        return indexer_y.Min();
    }

    T RangeY() const {
        return indexer_y.Range();
    }

    T MaxY() const {
        return indexer_y.Max();
    }
};


class DipoleFromTable : public CrossSection {
public:
private:
    std::map<Particle::ParticleType, Interpolator2D<double>> differential;
    std::map<Particle::ParticleType, Interpolator1D<double>> total;
    const std::set<Particle::ParticleType> primary_types = {Particle::ParticleType::NuE, Particle::ParticleType::NuMu, Particle::ParticleType::NuTau, Particle::ParticleType::NuEBar, Particle::ParticleType::NuMuBar, Particle::ParticleType::NuTauBar};
    double hnl_mass;
public:
    double GetHNLMass() const {return hnl_mass;};
    DipoleFromTable(double hnl_mass) : hnl_mass(hnl_mass) {};
    double TotalCrossSection(InteractionRecord const &) const;
    double TotalCrossSection(LeptonInjector::Particle::ParticleType primary, double energy, Particle::ParticleType target) const;
    double DifferentialCrossSection(InteractionRecord const &) const;
    double DifferentialCrossSection(Particle::ParticleType primary_type, double primary_energy, Particle::ParticleType target_type, double y) const;
    void SampleFinalState(InteractionRecord &, std::shared_ptr<LeptonInjector::LI_random>) const;

    std::vector<Particle::ParticleType> GetPossibleTargets() const;
    std::vector<Particle::ParticleType> GetPossibleTargetsFromPrimary(Particle::ParticleType primary_type) const;
    std::vector<Particle::ParticleType> GetPossiblePrimaries() const;
    std::vector<InteractionSignature> GetPossibleSignatures() const;
    std::vector<InteractionSignature> GetPossibleSignaturesFromParents(Particle::ParticleType primary_type, Particle::ParticleType target_type) const;
    void AddDifferentialCrossSectionFile(std::string filename, Particle::ParticleType target);
    void AddTotalCrossSectionFile(std::string filename, Particle::ParticleType target);
};

} // namespace LeptonInjector

#endif // LI_CrossSection_H

