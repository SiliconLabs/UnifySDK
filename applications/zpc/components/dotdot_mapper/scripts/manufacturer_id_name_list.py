import csv
import argparse

# This script takes in the Z-Wave Manufacturer ID list
# and outputs  map<Manufacturer ID, Manufacturer Name>

def parse(input, output):
    with open(input) as csvfile:
        reader = csv.reader(csvfile, delimiter=';')
        table_str = "// Following map is generated using manufacturer_id_name_list.py script\n"
        table_str += "#include <map>\n\n"
        table_str += "// map<Manufacturer ID, Manufacturer Name>\n"
        table_str += "static const std::map<uint32_t, std::string> manufacturer_id_name_map = {\n"
        all_lines = [row for row in reader]
        for row in all_lines[1:]:
            table_str +='{' + row[0] + ', "' + row[1].strip() + '"}, \n'
        table_str += "};\n"
        with open(output + 'manufacturer_id_name_list.inc', 'w') as out:
            out.write(
                "// Following map is generated using manufacturer_id_name_list.py script\n")
            out.write(table_str)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='''
    This script can generate manfacturer id namelookup tables.
    It takes a csv file as input, this file can be created by opening the
    Z-Wave manugacturer ID list xls file and exporting the "manufacturer_id_name_list" sheet as csv.''')
    parser.add_argument('-input', help='Input csv file',
                        default='../assets/manufacturer_id_name_list.csv')
    parser.add_argument(
        '-output', help='Path to store output files in. Files will be named manufacturer_id_name_list.inc', default='../src/')
    args = parser.parse_args()
    parse(args.input, args.output)
