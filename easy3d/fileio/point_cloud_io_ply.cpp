/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <easy3d/fileio/surface_mesh_io.h>

#include <unordered_map>

#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/types.h>
#include <easy3d/util/logging.h>


namespace easy3d {

	namespace io {

		namespace details {

			template <typename T, typename PropertyT>
			inline void add_properties(PointCloud* cloud, const std::vector<PropertyT>& properties)
			{
				for (const auto& p : properties) {
                    std::string name = p.name;
					if (name.find("v:") == std::string::npos)
						name = "v:" + name;
					auto prop = cloud->vertex_property<T>(name);
					prop.vector() = p;
				}
			}

		} // namespace details

		bool load_ply(const std::string& file_name, PointCloud* cloud) {
			std::vector<Element> elements;
			PlyReader reader;
			if (!reader.read(file_name, elements))
				return false;

            for (std::size_t i = 0; i < elements.size(); ++i) {
                const Element& e = elements[i];
                if (e.name == "vertex") {
                    cloud->resize(static_cast<unsigned int>(e.num_instances));
                    break;
                }
            }

			for (std::size_t i = 0; i < elements.size(); ++i) {
				const Element& e = elements[i];
                if (e.name == "vertex") {
                    details::add_properties<vec3>(cloud, e.vec3_properties);
                    details::add_properties<float>(cloud, e.float_properties);
                    details::add_properties<int>(cloud, e.int_properties);
                    details::add_properties< std::vector<int> >(cloud, e.int_list_properties);
                    details::add_properties< std::vector<float> >(cloud, e.float_list_properties);
                }
                else if (e.name == "edge")
                    std::cerr << "The PointCloud has edge information (ignored). Is it a graph?" << std::endl;
                else if (e.name == "face")
                    std::cerr << "The PointCloud has face information (ignored). Is it a mesh?" << std::endl;
                else
                    std::cerr << "The PointCloud has unknown element: " << e.name << " (ignored)" << std::endl;
			}

			return true;
		}


		namespace details {

			template <typename T>
			inline void collect_properties(const PointCloud* cloud, std::vector< GenericProperty<T> >& properties) {
				const auto& all_properties = cloud->vertex_properties();
				for (auto name : all_properties) {
					auto prop = cloud->get_vertex_property<T>(name);
					if (prop) {
						if (name.substr(0, 2) == "v:")
							name = name.substr(2, name.length() - 1);
						properties.emplace_back(GenericProperty<T>("vertex", name, prop.vector()));
					}
				}
			}

		} // namespace details

		bool save_ply(const std::string& file_name, const PointCloud* cloud, bool binary/* = true*/) {
			if (!cloud || cloud->n_vertices() == 0) {
				std::cerr << "empty point cloud data" << std::endl;
				return false;
			}

			std::size_t num = cloud->n_vertices();
			Element e("vertex", num);

			details::collect_properties(cloud, e.vec3_properties);
			details::collect_properties(cloud, e.float_properties);
			details::collect_properties(cloud, e.int_properties);
			details::collect_properties(cloud, e.int_list_properties);
			details::collect_properties(cloud, e.float_list_properties);

			std::vector<Element> elements;
            elements.emplace_back(e);

            DLOG_IF(WARNING, !binary) << "you're writing an ASCII ply file. Use binary format for better performance" << std::endl;

			PlyWriter writer;
			return writer.write(file_name, elements, "", binary);
		}


	} // namespace io

} // namespace easy3d
