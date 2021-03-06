/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/**
    @file Json.cpp
*/

#include <File.hpp>
#include <Json.hpp>

void Json::clear()
{
    type_ = TYPE_NULL;
    data_.object.reset();
    data_.array.reset();
    data_.string.reset();
}

void Json::read(const std::string &filename)
{
    std::string data = File::read(filename);
    deserialize(data);
}

void Json::write(const std::string &filename, size_t indent)
{
    std::string data = serialize(indent);
    File::write(filename, data);
}

std::string Json::serialize(size_t indent) const
{
    std::ostringstream out;

    if (indent == 0)
    {
        serialize(out);
    }
    else
    {
        std::string str;
        str.resize(indent);
        for (size_t i = 0; i < str.size(); i++)
        {
            str[i] = ' ';
        }
        serialize(out, "", str);
    }

    return out.str();
}

void Json::deserialize(const std::string &in)
{
    deserialize(in.c_str(), in.size());
}

void Json::deserialize(const char *in, size_t n)
{
    size_t i = 0;
    deserialize(*this, in, n, i);
}

void Json::serialize(std::ostringstream &out) const
{
    size_t i = 0;
    size_t n;

    switch (type_)
    {
        case TYPE_OBJECT:
            out << "{";
            n = data_.object->size();
            for (auto const &it : *data_.object)
            {
                out << "\"" << it.first << "\":";
                it.second.serialize(out);
                i++;
                if (i < n)
                {
                    out << ",";
                }
            }
            out << "}";
            break;

        case TYPE_ARRAY:
            out << "[";
            n = data_.array->size();
            for (auto const &it : *data_.array)
            {
                it.serialize(out);
                i++;
                if (i < n)
                {
                    out << ",";
                }
            }
            out << "]";
            break;

        case TYPE_STRING:
            out << "\"" << *data_.string << "\"";
            break;

        case TYPE_NUMBER:
            out << data_.number;
            break;

        case TYPE_TRUE:
            out << "true";
            break;

        case TYPE_FALSE:
            out << "false";
            break;

        case TYPE_NULL:
        default:
            out << "null";
            break;
    }
}

void Json::serialize(std::ostringstream &out,
                     const std::string &indent,
                     const std::string &indent_plus) const
{
    std::string indent2;
    size_t i = 0;
    size_t n;

    switch (type_)
    {
        case TYPE_OBJECT:
            out << "{\n";
            indent2 = indent + indent_plus;
            n = data_.object->size();
            for (auto const &it : *data_.object)
            {
                out << indent2 << "\"" << it.first << "\":";
                it.second.serialize(out, indent2, indent_plus);
                i++;
                if (i < n)
                {
                    out << ",\n";
                }
            }
            out << "\n" << indent << "}";
            break;

        case TYPE_ARRAY:
            out << "[";
            n = data_.array->size();
            for (auto const &it : *data_.array)
            {
                it.serialize(out, indent, indent_plus);
                i++;
                if (i < n)
                {
                    out << ",";
                }
            }
            out << "]";
            break;

        case TYPE_STRING:
            out << "\"" << *data_.string << "\"";
            break;

        case TYPE_NUMBER:
            out << data_.number;
            break;

        case TYPE_TRUE:
            out << "true";
            break;

        case TYPE_FALSE:
            out << "false";
            break;

        case TYPE_NULL:
        default:
            out << "null";
            break;
    }
}

void Json::deserialize(Json &obj, const char *in, size_t n, size_t &i)
{
    size_t str_start;
    size_t str_len;
    size_t array_index;
    std::string str;

    enum State
    {
        STATE_VALUE,
        STATE_OBJECT,
        STATE_OBJECT_PAIR,
        STATE_ARRAY,
        STATE_STRING,
        STATE_STRING_NAME,
        STATE_STRING_VALUE,
        STATE_NUMBER,
        STATE_TYPE
    };
    int state = STATE_VALUE;
    int next;

    while (i < n)
    {
        switch (state)
        {
            case STATE_VALUE:
                if (in[i] == '{')
                {
                    obj.create_object();
                    state = STATE_OBJECT;
                }
                else if (in[i] == '[')
                {
                    obj.create_array();
                    state = STATE_ARRAY;
                    array_index = 0;
                }
                else if (in[i] == '\"')
                {
                    str_start = i + 1;
                    state = STATE_STRING;
                    next = STATE_STRING_VALUE;
                }
                else if (in[i] == '-' || (in[i] > 47 && in[i] < 58))
                {
                    str_start = i;
                    state = STATE_NUMBER;
                }
                else if (in[i] > 96 && in[i] < 123)
                {
                    str_start = i;
                    state = STATE_TYPE;
                }
                break;

            case STATE_OBJECT:
                if (in[i] == '\"')
                {
                    str_start = i + 1;
                    state = STATE_STRING;
                    next = STATE_STRING_NAME;
                }
                else if (in[i] == '}')
                {
                    i++;
                    return;
                }
                break;

            case STATE_STRING_NAME:
                if (in[i] == ':')
                {
                    state = STATE_OBJECT_PAIR;
                }
                break;

            case STATE_OBJECT_PAIR:
                // parse object pair value
                deserialize(obj[str], in, n, i);
                i--;
                state = STATE_OBJECT;
                break;

            case STATE_STRING:
                // TBD extra characters
                if (in[i] == '\"')
                {
                    str_len = i - str_start;
                    str.resize(str_len);
                    for (size_t j = 0; j < str_len; j++)
                    {
                        str[j] = in[str_start + j];
                    }
                    state = next;
                }
                break;

            case STATE_STRING_VALUE:
                obj.create_string(str);
                return;

            case STATE_NUMBER:
                // TBD number format
                if (!(in[i] == '.' || (in[i] > 47 && in[i] < 58)))
                {
                    str_len = i - str_start;
                    str.resize(str_len);
                    for (size_t j = 0; j < str_len; j++)
                    {
                        str[j] = in[str_start + j];
                    }
                    obj.create_number(std::stod(str));
                    return;
                }
                break;

            case STATE_ARRAY:
                if (in[i] > 32 && in[i] != ']')
                {
                    // parse next array element
                    deserialize(obj[array_index], in, n, i);
                    array_index++;
                }

                if (in[i] == ']')
                {
                    i++;
                    return;
                }
                break;

            case STATE_TYPE:
                if (!(in[i] > 96 && in[i] < 123))
                {
                    str_len = i - str_start;
                    str.resize(str_len);
                    for (size_t j = 0; j < str_len; j++)
                    {
                        str[j] = in[str_start + j];
                    }

                    if (str == "null")
                    {
                        obj.create_type(TYPE_NULL);
                    }
                    else if (str == "false")
                    {
                        obj.create_type(TYPE_FALSE);
                    }
                    else
                    {
                        obj.create_type(TYPE_TRUE);
                    }

                    return;
                }
                break;

            default:
                break;
        }

        i++;
    }
}
