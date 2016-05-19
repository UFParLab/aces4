/*
 * create_map.h
 *
 *  Created on: Sep 18, 2015
 *   copied from #83 at
 *   http://stackoverflow.com/questions/138600/initializing-a-static-stdmapint-int-in-c
 *
 *   Usage
 *   std::map mymap = create_map<int, int >(1,2)(3,4)(5,6);
 *
 *   Notice:  no comma between the entries.
 */

#ifndef CREATE_MAP_H_
#define CREATE_MAP_H_


template <typename T, typename U>
class create_map
{
private:
    std::map<T, U> m_map;
public:
    create_map(const T& key, const U& val)
    {
        m_map[key] = val;
    }

    create_map<T, U>& operator()(const T& key, const U& val)
    {
        m_map[key] = val;
        return *this;
    }

    operator std::map<T, U>()
    {
        return m_map;
    }
};


#endif /* CREATE_MAP_H_ */
