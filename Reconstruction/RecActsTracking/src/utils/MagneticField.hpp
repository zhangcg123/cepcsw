// STL
#include <memory>
#include <variant>
#include <vector>

// acts
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/InterpolatedBFieldMap.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/NullBField.hpp"
#include "Acts/Utilities/Grid.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/Utilities/detail/Axis.hpp"
#include "Acts/Utilities/detail/AxisFwd.hpp"
#include "Acts/Utilities/detail/grid_helper.hpp"

/// The ScalableBField-specific magnetic field context.
struct ScalableBFieldContext
{ Acts::ActsScalar scalor = 1.; };

/// A constant magnetic field that is scaled depending on the event context.
class ScalableBField final : public Acts::MagneticFieldProvider
{
    public:
        struct Cache
        {
            Acts::ActsScalar scalor = 1.;
            /// @brief constructor with context
            Cache(const Acts::MagneticFieldContext& mctx)
            { scalor = mctx.get<const ScalableBFieldContext>().scalor; }
        };

        /// @brief construct constant magnetic field from field vector
        ///
        /// @param [in] B magnetic field vector in global coordinate system
        explicit ScalableBField(Acts::Vector3 B) : m_BField(std::move(B)) {}

        /// @brief construct constant magnetic field from components
        ///
        /// @param [in] Bx magnetic field component in global x-direction
        /// @param [in] By magnetic field component in global y-direction
        /// @param [in] Bz magnetic field component in global z-direction
        ScalableBField(Acts::ActsScalar Bx = 0, Acts::ActsScalar By = 0, Acts::ActsScalar Bz = 0)
        : m_BField(Bx, By, Bz) {}

        /// @brief retrieve magnetic field value
        ///
        /// @param [in] position global position
        /// @param [in] cache Cache object (is ignored)
        /// @return magnetic field vector
        ///
        /// @note The @p position is ignored and only kept as argument to provide
        ///       a consistent interface with other magnetic field services.
        Acts::Result<Acts::Vector3> getField(
            const Acts::Vector3& /*position*/,
            MagneticFieldProvider::Cache& gCache) const override
        {
            Cache& cache = gCache.as<Cache>();
            return Acts::Result<Acts::Vector3>::success(m_BField * cache.scalor);
        }

        /// @brief retrieve magnetic field value & its gradient
        ///
        /// @param [in]  position   global position
        /// @param [out] derivative gradient of magnetic field vector as (3x3)
        /// matrix
        /// @param [in] cache Cache object (is ignored)
        /// @return magnetic field vector
        ///
        /// @note The @p position is ignored and only kept as argument to provide
        ///       a consistent interface with other magnetic field services.
        /// @note currently the derivative is not calculated
        /// @todo return derivative
        Acts::Result<Acts::Vector3> getFieldGradient(
            const Acts::Vector3& /*position*/, Acts::ActsMatrix<3, 3>& /*derivative*/,
            MagneticFieldProvider::Cache& gCache) const override
        {
            Cache& cache = gCache.as<Cache>();
            return Acts::Result<Acts::Vector3>::success(m_BField * cache.scalor);
        }

        Acts::MagneticFieldProvider::Cache makeCache(
            const Acts::MagneticFieldContext& mctx) const override
        {
            return Acts::MagneticFieldProvider::Cache(std::in_place_type<Cache>, mctx);
        }

        /// @brief check whether given 3D position is inside look-up domain
        ///
        /// @param [in] position global 3D position
        /// @return @c true if position is inside the defined look-up grid,
        ///         otherwise @c false
        /// @note The method will always return true for the constant B-Field
        bool isInside(const Acts::Vector3& /*position*/) const { return true; }

        /// @brief update magnetic field vector from components
        ///
        /// @param [in] Bx magnetic field component in global x-direction
        /// @param [in] By magnetic field component in global y-direction
        /// @param [in] Bz magnetic field component in global z-direction
        void setField(double Bx, double By, double Bz) { m_BField << Bx, By, Bz; }

        /// @brief update magnetic field vector
        ///
        /// @param [in] B magnetic field vector in global coordinate system
        void setField(const Acts::Vector3& B) { m_BField = B; }

    private:
        /// magnetic field vector
        Acts::Vector3 m_BField;
}; // ScalableBField

using InterpolatedMagneticField2 = Acts::InterpolatedBFieldMap<
    Acts::Grid<Acts::Vector2, Acts::detail::EquidistantAxis,
               Acts::detail::EquidistantAxis>>;

using InterpolatedMagneticField3 = Acts::InterpolatedBFieldMap<
    Acts::Grid<Acts::Vector3, Acts::detail::EquidistantAxis,
               Acts::detail::EquidistantAxis, Acts::detail::EquidistantAxis>>;
