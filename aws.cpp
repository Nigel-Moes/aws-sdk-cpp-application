#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/model/DescribeInstancesRequest.h>

using namespace std;

#define REGIONS "regions.txt"

Aws::String getInput(string& inputName)
{
    Aws::String input;
    while (true) 
    {
        cout << "Enter " + inputName + ": ";
        cin >> input;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Invalid input. Please try again: "; 
        }
        else
        {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
    }
    return input;
}

Aws::Auth::AWSCredentials setCredentials()
{
    // Get the AWS Access Key ID
    string accessKeyIDString {"AWS Access Key ID"};
    Aws::String accessKeyID {getInput(accessKeyIDString)};

    // Get the AWS Secret Access Key
    string secretAccessKeyString {"AWS Secret Access Key"};
    Aws::String secretAccessKey {getInput(secretAccessKeyString)};

    // Set the credential
    Aws::Auth::AWSCredentials credentials(accessKeyID, secretAccessKey);

    // Clear the input
    memset(&accessKeyID[0], 0, accessKeyID.length());
    accessKeyID.clear();
    memset(&secretAccessKey[0], 0, secretAccessKey.length());
    secretAccessKey.clear();

    // Return the credentials
    return credentials;
}

vector<string> readRegions()
{
    vector<string> regions;
    string line;
    ifstream file(REGIONS);

    if (file.is_open())
    {
        while (getline(file, line))
        {
            regions.push_back(line);
        }
        file.close();
    }
    else cerr << "Unable to open file" << endl;

    return regions;
}

void displayRegions(vector<string>& regions)
{
    cout << "Please choose a region (number): " << endl;
    for (int i {0}; i < regions.size(); i++)
    {
        cout << i+1 << ". " << regions[i] << endl;
    }
}


void setRegion(Aws::Client::ClientConfiguration& clientConfig)
{
    vector<string> regions {readRegions()};
    displayRegions(regions);

    string regionNumberString {"Region Number"};
    Aws::String regionNumber {getInput(regionNumberString)};
    int choice {stoi(regionNumber)};

    while (choice < 1 || choice > regions.size())
    {
        cout << "Invalid choice. Please try again." << endl;
        regionNumber = getInput(regionNumberString);
        choice = stoi(regionNumber);
    }

    clientConfig.region = regions[choice-1];
}

int main() 
{
    // Initialize the SDK
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    // Set the credentials
    Aws::Auth::AWSCredentials credentials {setCredentials()};

    // Set the region
    Aws::Client::ClientConfiguration config;
    setRegion(config);

    // Create an EC2 client
    Aws::EC2::EC2Client ec2Client(credentials, config);

    // Get EC2 instances
    Aws::EC2::Model::DescribeInstancesRequest request;
    auto start {std::chrono::steady_clock::now()};
    auto outcome = ec2Client.DescribeInstances(request);
    auto end {std::chrono::steady_clock::now()};
    chrono::duration<double> elapsed_seconds {end - start};

    // Display the results
    cout << "Latency: " << elapsed_seconds.count() << " seconds" << endl;
    if (outcome.IsSuccess()) 
    {
        cout << "Found " << outcome.GetResult().GetReservations().size() << " instances" << endl;
        for (auto &reservation : outcome.GetResult().GetReservations()) 
        {
            for (auto &instance : reservation.GetInstances()) 
            {
                cout << instance.GetInstanceId() << endl;
            }
        }
    }
    else 
    {
        cerr << "Error: " << outcome.GetError().GetMessage() << endl;
    }

    // Close the API
    Aws::ShutdownAPI(options);
    return 0;
}
