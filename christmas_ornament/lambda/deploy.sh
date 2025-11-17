#!/bin/bash
# Quick deploy script for ESP8266 Remote Control Lambda function

echo "🚀 Deploying ESP8266 Remote Control to AWS Lambda"
echo ""

# Check if AWS CLI is installed
if ! command -v aws &> /dev/null; then
    echo "❌ AWS CLI not found. Installing..."
    echo "Visit: https://aws.amazon.com/cli/"
    exit 1
fi

# Check if logged in
if ! aws sts get-caller-identity &> /dev/null; then
    echo "❌ Not logged in to AWS. Please run: aws configure"
    exit 1
fi

echo "✓ AWS CLI configured"
echo ""

# Get user input
read -p "Enter Lambda function name (default: esp8266-remote-control): " FUNCTION_NAME
FUNCTION_NAME=${FUNCTION_NAME:-esp8266-remote-control}

read -p "Enter AWS region (default: us-east-1): " REGION
REGION=${REGION:-us-east-1}

echo ""
echo "📦 Creating deployment package..."

# Create deployment package
zip -q function.zip esp8266-remote-control.js

echo "✓ Package created"
echo ""

# Check if function exists
if aws lambda get-function --function-name "$FUNCTION_NAME" --region "$REGION" &> /dev/null; then
    echo "📝 Updating existing function..."
    aws lambda update-function-code \
        --function-name "$FUNCTION_NAME" \
        --zip-file fileb://function.zip \
        --region "$REGION" &> /dev/null
    
    if [ $? -eq 0 ]; then
        echo "✓ Function updated"
    else
        echo "❌ Update failed"
        exit 1
    fi
else
    echo "🆕 Creating new function..."
    
    # Create IAM role (simplified - use existing role in production)
    aws iam create-role \
        --role-name lambda-execution-role \
        --assume-role-policy-document '{"Version":"2012-10-17","Statement":[{"Effect":"Allow","Principal":{"Service":"lambda.amazonaws.com"},"Action":"sts:AssumeRole"}]}' \
        &> /dev/null
    
    # Attach basic execution policy
    aws iam attach-role-policy \
        --role-name lambda-execution-role \
        --policy-arn arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole \
        &> /dev/null
    
    # Get account ID
    ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
    
    ROLE_ARN="arn:aws:iam::${ACCOUNT_ID}:role/lambda-execution-role"
    
    # Wait for role to be ready
    sleep 5
    
    # Create function
    aws lambda create-function \
        --function-name "$FUNCTION_NAME" \
        --runtime nodejs20.x \
        --role "$ROLE_ARN" \
        --handler esp8266-remote-control.handler \
        --zip-file fileb://function.zip \
        --region "$REGION" \
        --timeout 10 \
        --memory-size 128 &> /dev/null
    
    if [ $? -eq 0 ]; then
        echo "✓ Function created"
    else
        echo "❌ Creation failed"
        echo "Try deploying via AWS Console instead"
        exit 1
    fi
fi

echo ""

# Create or update Function URL
echo "🔗 Setting up Function URL..."

aws lambda create-function-url-config \
    --function-name "$FUNCTION_NAME" \
    --auth-type NONE \
    --cors '{"AllowOrigins":["*"],"AllowMethods":["GET","POST","OPTIONS"],"AllowHeaders":["Content-Type"]}' \
    --region "$REGION" &> /dev/null

# Get Function URL
FUNCTION_URL=$(aws lambda get-function-url-config \
    --function-name "$FUNCTION_NAME" \
    --region "$REGION" \
    --query FunctionUrl \
    --output text)

echo "✓ Function URL created"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Deployment complete!"
echo ""
echo "📌 Your Lambda Function URL:"
echo "   $FUNCTION_URL"
echo ""
echo "📝 Next steps:"
echo "   1. Update ESP8266 code with the URL above"
echo "   2. Update public_control_example.html"
echo "   3. Upload ESP8266 sketch"
echo ""
echo "🧪 Test it:"
echo "   curl -X POST $FUNCTION_URL/device/command \\"
echo "     -H 'Content-Type: application/json' \\"
echo "     -d '{\"deviceId\":\"ESP8266_001\",\"command\":\"led1_on\"}'"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Cleanup
rm function.zip
