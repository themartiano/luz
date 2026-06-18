#pragma once

#include "Color.hpp"
#include "Materials/Material.hpp"
#include "Vector3.hpp"
#include <cstdint>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

class	Scene;

class	CausticPhotonMap
{
	public:
		struct	Photon
		{
			Vector3	position;
			Vector3	normal;
			Vector3	incomingDirection;
			Color	flux;
		};

		void	build(Scene& scene, std::uint32_t renderSeed);
		void	clear(void);
		Color	estimate(const HitRecord& hitRecord, const ScatterRecord& scatterRecord) const;
		std::size_t	photonCount(void) const;
		double	radiusMeters(void) const;
		double	radiusSceneUnits(void) const;

	private:
		struct	GridKey
		{
			long long	x = 0;
			long long	y = 0;
			long long	z = 0;

			bool	operator==(const GridKey& other) const
			{
				return (this->x == other.x && this->y == other.y && this->z == other.z);
			}
		};

		struct	GridKeyHash
		{
			std::size_t	operator()(const GridKey& key) const;
		};

		void	storePhoton(const Photon& photon);
		void	rebuildGrid(void);
		GridKey	gridKey(const Vector3& position) const;

		std::vector<Photon>	_photons;
		std::unordered_map<GridKey, std::vector<std::size_t>, GridKeyHash>	_grid;
		double	_radiusMeters = 0.0;
		double	_radiusSceneUnits = 0.0;
		double	_gatherAreaSquareMeters = 0.0;
};
